#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <fstream>

#include "mpi.h"
#include <omp.h>

#define INFINITE 999999999

int main(int argc, char **argv){

    int taskNum, rank;
	MPI_Comm SURVIVAL_COMM;
	MPI_Group WORLD_GROUP, SURVIVAL_GROUP;

    MPI_Init(&argc, &argv);

    //determine the number of tasks in the group & the rank of each task within the communicator
	MPI_Comm_size(MPI_COMM_WORLD, &taskNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //read input file
    std::ifstream inputFile(argv[1], std::ios::in | std::ios::binary);
    int vertexNum, edgeNum;
    inputFile.read((char*)&vertexNum, 4);
    inputFile.read((char*)&edgeNum, 4);

    //If the number of the problem size is larger than the number of tasks, finalize the unnecessary tasks
	//It must create a new group before creating a new communicator!!
    SURVIVAL_COMM = MPI_COMM_WORLD;
	if(vertexNum < taskNum){
		MPI_Comm_group(MPI_COMM_WORLD, &WORLD_GROUP);
		int surviveRange[][3] = { {0, vertexNum-1, 1} }; //0~vertex number-1, displacement 1
		MPI_Group_range_incl(WORLD_GROUP, 1, surviveRange, &SURVIVAL_GROUP);
		//create new communicator & broadcast within new group
		//other task out of new group will get MPI_COMM_NULL in SURVIVAL_WORLD
		MPI_Comm_create(MPI_COMM_WORLD, SURVIVAL_GROUP, &SURVIVAL_COMM);
		if(SURVIVAL_COMM == MPI_COMM_NULL){
			MPI_Finalize();
			exit(0);
		}
		taskNum = vertexNum;
	}

    //allocate problem size to each task
	int remainder = vertexNum % taskNum;
	int quotient = vertexNum / taskNum;
	int begin = rank * quotient; //the first vertex of a rank
	int taskVertexNum = quotient;
	if(rank < remainder){
		++taskVertexNum;
		begin += rank;
	}else{
		begin += remainder;
	}
	int end = begin + taskVertexNum;


    int* distance_rowk = new int[vertexNum];
    distance_rowk[0] = 0;
    #pragma omp parallel for num_threads(12) schedule(static)
    for(int i=1; i<vertexNum; ++i) distance_rowk[i] = INFINITE;

    // new distance array for each task
    int* distance = new int[taskVertexNum * vertexNum];
    #pragma omp parallel for num_threads(12) schedule(static) collapse(2)
    for(int i=begin; i<end; ++i){
        for(int j=0; j<vertexNum; ++j){
            if( i == j) distance[ (i-begin) * vertexNum + j ] = 0;
            else distance[ (i-begin) * vertexNum + j ] = INFINITE;
        }
    }

    int source, destination, weight;
    while( inputFile.read((char*)&source, 4) ){
        inputFile.read((char*)&destination, 4);
        inputFile.read((char*)&weight, 4);
        if(source == 0) distance_rowk[destination] = weight;
        if(source >= begin && source < end) distance[ (source - begin) * vertexNum + destination ] = weight;
    }
    inputFile.close();
    

    //do k=0
    int index;
    #pragma omp parallel for num_threads(12) schedule(static) collapse(2) private(index)
    for(int i=begin; i<end; ++i){
        for(int j=0; j<vertexNum; ++j){
            index = (i-begin) * vertexNum + j;
            distance[ index ] = std::min( distance[ index ], distance[ index - j ] + distance_rowk[j] );
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


    //Floyd Warshall
    int root, offset;
    for(int k=1; k<vertexNum; ++k){
        //recv k-th row distance to other process(es)
        if( k >= begin && k < end ){
            root = rank;
            MPI_Bcast( distance + (k-begin) * vertexNum, vertexNum, MPI_INT, root, SURVIVAL_COMM );
            //calculate shortest path
            #pragma omp parallel for num_threads(12) schedule(static) collapse(2) private(offset)
            for(int i=0; i<taskVertexNum; ++i){
                for(int j=0; j<vertexNum; ++j){
                    offset = i * vertexNum;
                    distance[ offset + j ] = std::min( distance[ offset + j ], distance[ offset + k ] + distance[ (k-begin) * vertexNum + j ] );
                }
            }
        }else{
            #pragma omp parallel for num_threads(12) schedule(static)
            for(int i=0; i<taskNum; ++i){
                if(displacement[i]<=k && displacement[i]+count[i]>k) root = i;
            }
            MPI_Bcast( distance_rowk, vertexNum, MPI_INT, root, SURVIVAL_COMM );
            //calculate shortest path
            #pragma omp parallel for num_threads(12) schedule(static) collapse(2) private(offset)
            for(int i=0; i<taskVertexNum; ++i){
                for(int j=0; j<vertexNum; ++j){
                    offset = i * vertexNum;
                    distance[ offset + j ] = std::min( distance[ offset + j ], distance[ offset + k ] + distance_rowk[j] );
                }
            }
        }
    }

    // write result into file
    MPI_File outputFile;
	MPI_File_open(SURVIVAL_COMM, argv[2], MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &outputFile);
    offset = 4 * begin * vertexNum;
    MPI_File_write_at(outputFile, offset, distance, taskVertexNum * vertexNum, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_close(&outputFile);

    
    delete[] count;
    delete[] displacement;
    delete[] distance;
    delete[] distance_rowk;
	MPI_Finalize();
    return 0;
}