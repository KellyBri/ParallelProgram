#include <iostream>
#include <cstring>
#include <cstdlib>
#include <mpi.h>
#include <png.h>
#define PNG_NO_SETJMP
#define MAX_ITER 10000

typedef struct {
    double x;
    double y;
}COMPLEX;


void write_png(const char* filename, const int width, const int height, const int* buffer) {
    FILE* fp = fopen(filename, "wb");
    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png_ptr, info_ptr);
    size_t row_size = 3 * width * sizeof(png_byte);
    png_bytep row = (png_bytep)malloc(row_size);
    for (int y = 0; y < height; ++y) {
        memset(row, 0, row_size);
        for (int x = 0; x < width; ++x) {
            int p = buffer[(height - 1 - y) * width + x];
            png_bytep color = row + x * 3;
            if (p != MAX_ITER) {
                if (p & 16) {
                    color[0] = 240;
                    color[1] = color[2] = p % 16 * 16;
                } else {
                    color[0] = p % 16 * 16;
                }
            }
        }
        png_write_row(png_ptr, row);
    }
    free(row);
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
}

int main(int argc, char** argv){

	//check the number of parameter
	//If it wouldn't meet the require, show error message and terminate the program.
	if(argc != 9){ 
        std::cout<<"Error number of parameter. Terminating."<<std::endl; 
        return 0;
    }

    //parse parameter
    int threadNum = atoi(argv[1]);  //useless
    //[x_leftBound, x_rightBound)
    int x_point = atoi(argv[6]);
    double x_leftBound = atof(argv[2]);
    double x_rightBound = atof(argv[3]);
    //[y_lowerBound, y_upperBound)
    int y_point = atoi(argv[7]);
    double y_lowerBound = atof(argv[4]);
    double y_upperBound = atof(argv[5]);
    std::string fileName = argv[8];

    
    int rc, taskNum, rank; //taskNum: number follow behind -n
	int globalProblemSize = x_point * y_point;

	//initialize the MPI execution environment
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS){
		std::cout<<"Error starting MPI program. Terminating."<<std::endl;
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	//determine the number of tasks in the group & the rank of each task within the communicator
    MPI_Comm SURVIVAL_COMM = MPI_COMM_WORLD;
	MPI_Group WORLD_GROUP, SURVIVAL_GROUP;
	MPI_Comm_size(MPI_COMM_WORLD, &taskNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //If the number of the problem size is larger than the number of tasks, finalize the unnecessary tasks
	//It must create a new group before creating a new communicator!!
	if(globalProblemSize < taskNum){
		MPI_Comm_group(MPI_COMM_WORLD, &WORLD_GROUP);
		int surviveRange[][3] = { {0, globalProblemSize-1, 1} }; //0~problem size-1, displacement 1
		MPI_Group_range_incl(WORLD_GROUP, 1, surviveRange, &SURVIVAL_GROUP);
		//create new communicator & broadcast within new group
		//other task out of new group will get MPI_COMM_NULL in SURVIVAL_WORLD
		MPI_Comm_create(MPI_COMM_WORLD, SURVIVAL_GROUP, &SURVIVAL_COMM);
		if(SURVIVAL_COMM == MPI_COMM_NULL){
			MPI_Finalize();
			exit(0);
		}
		taskNum = globalProblemSize;
	}

    //allocate problem size to each task
	int remainder = globalProblemSize % taskNum;
	int quotient = globalProblemSize / taskNum;
	int taskProblemSize = quotient;
	if(rank < remainder) ++taskProblemSize;
	

    

    //random x, y, and send them to each task
    int *globalProblem = new int[globalProblemSize];
    
    for(int i=0; i<globalProblemSize; ++i){
        globalProblem[i] = i;
    }
    //random the coordinate
    if(rank == 0){
        srand((unsigned)time(NULL));
        for(int i=0; i<globalProblemSize; ++i){
            int r = rand() % globalProblemSize;
            std::swap(globalProblem[i], globalProblem[r]);
        }
    }


    int *displacement = new int[taskNum];
    int *count = new int[taskNum];
    for(int i = 0; i<taskNum; ++i){
        if(i != 0) displacement[i] = displacement[i-1] + count[i-1];
        else displacement[i] = 0;
        
        if(i < remainder) count[i] = quotient + 1;
        else count[i] = quotient;
    }

    int *problem = new int[taskProblemSize];
    int *iteration = new int[taskProblemSize];
    MPI_Scatterv(globalProblem, count, displacement, MPI_INT, problem, taskProblemSize, MPI_INT, 0, SURVIVAL_COMM);
    // for(int i = 0; i<taskProblemSize; ++i){
    //     printf("I'm rank %d, test %d = %d\n", rank, i, problem[i]);
    // }


    //MPI_Bcast(problem, globalProblemSize, MPI_INT, 0, SURVIVAL_COMM);

    /* Mandelbort set */
    double x_increment = (x_rightBound - x_leftBound) / x_point;
    double y_increment = (y_upperBound - y_lowerBound) / y_point;
    
    #pragma omp parallel for num_threads(threadNum) schedule(dynamic,16)
    for(int i = 0; i < count[rank]; ++i){
        // printf("I'm rank %d: %d\n", rank, problem[i]);
        //coordinate
        int y = problem[i] / x_point;
        int x = problem[i] % x_point;
        // printf("I'm rank %d: (%d, %d)\n", rank, x, y);
        COMPLEX Z, C;
        C.x = x_leftBound + x_increment * x;
        C.y = y_lowerBound + y_increment * y;
        // printf("I'm rank %d: (%.2f, %.2f)\n", rank, C.x, C.y);
        Z.x = 0;
        Z.y = 0;
        int j = 0;
        double length = 0;
        while(j < MAX_ITER && length <= 4){
            double temp = Z.x * Z.x - Z.y * Z.y + C.x;
            Z.y = 2 * Z.x * Z.y + C.y;
            Z.x = temp;
            length = Z.x * Z.x + Z.y * Z.y;
            ++j;
        }
        iteration[i] = j;
        //printf("I'm rank %d: (%.2f, %.2f) = %d\n",rank, C.x, C.y, iteration[i]);
    }
    int *pixel = new int[globalProblemSize]{0};
    int *globalIteration = new int[globalProblemSize];
    MPI_Gatherv(iteration, taskProblemSize, MPI_INT, globalIteration, count, displacement, MPI_INT, 0, SURVIVAL_COMM);

                
    // MPI_Allreduce(iteration, pixel, globalProblemSize, MPI_INT, MPI_SUM, SURVIVAL_COMM);
    
    if(rank == 0){
        for(int i=0; i<globalProblemSize; ++i){
            int y = globalProblem[i] / x_point;
            int x = globalProblem[i] % x_point;
            pixel[y*x_point + x] = globalIteration[i];
            //printf("(%d, %d) = %d\n",x,y, globalIteration[i]);
        }
        write_png(argv[8], x_point, y_point, pixel);
    }

    delete[] iteration;
    delete[] globalIteration;
    delete[] problem;
    delete[] globalProblem;
    delete[] displacement;
    delete[] count;
    delete[] pixel;
    
    MPI_Finalize();

    return 0;
}