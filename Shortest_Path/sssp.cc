#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include "mpi.h"

#define INFINITE 2147483647


enum{ WHITE, BLACK };
enum{ COLOR_TAG, DATA_TAG, TERMINATE_TAG };

typedef struct{
    int source;
    int destination;
    int weight;
}EDGE;

bool compareID(EDGE &a, EDGE &b){ return a.source < b.source; }


int main(int argc, char **argv){

    int taskNum, rank;
	MPI_Comm SURVIVAL_COMM;
	MPI_Group WORLD_GROUP, SURVIVAL_GROUP;

    MPI_Init(&argc, &argv);

    //determine the number of tasks in the group & the rank of each task within the communicator
	MPI_Comm_size(MPI_COMM_WORLD, &taskNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //read input file
    int vertexNum, edgeNum;
    std::ifstream inputFile(argv[1], std::ios::in | std::ios::binary);
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
	}else begin += remainder;
	int end = begin + taskVertexNum;

    //read edges
    std::vector<EDGE> edge;
    int source, destination, weight;
    for(int i=0; i<edgeNum; ++i){
        inputFile.read((char*)&source, 4);
        inputFile.read((char*)&destination, 4);
        inputFile.read((char*)&weight, 4);
        // printf("%d, %d, %d\n", source, destination, weight);
        if( source >= begin && source < end ) edge.push_back( {source, destination, weight} );
    }
    inputFile.close();
    std::sort(edge.begin(), edge.end(), compareID);

    //record the range of source ID in each task 
    int *displacement = new int[taskNum];
    int *count = new int[taskNum];
    for(int i = 0; i<taskNum; ++i){
        if(i != 0) displacement[i] = displacement[i-1] + count[i-1];
        else displacement[i] = 0;
        
        if(i < remainder) count[i] = quotient + 1;
        else count[i] = quotient;
    }

    // for(auto it:edge)
    //     printf("Rank %d ---- (%d\t, %d)\t%d\n", rank, it.source, it.destination, it.weight);


    // /*************************** Moore's single source shortest path ***************************/
    //initial distance
    int *distance = new int[taskVertexNum];
    for(int i=0; i<taskVertexNum; ++i) distance[i] = INFINITE;

    int cost, dest;
    int color = WHITE;
    MPI_Request request;
    if(rank == 0){
        distance[0] = 0;
        color = BLACK;

        for(auto it:edge){
            if(it.source == 0){
                cost = it.weight;
                for(int i=0; i<taskNum; ++i){
                    if( it.destination >= displacement[i] && it.destination < displacement[i]+count[i]) dest = i;
                }
                if( dest != 0 ) MPI_Isend( &cost, 1, MPI_INT, dest, DATA_TAG, SURVIVAL_COMM, &request );
                else distance[ it.destination - begin ] = cost;
            }else break;
        }
        
        MPI_Isend( &color, 1, MPI_INT, rank + 1, COLOR_TAG, SURVIVAL_COMM, &request );
        color = WHITE;
    }


   

    MPI_Status status;
    while (true){
        MPI_Recv( &cost, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, SURVIVAL_COMM, &status);
        printf("[Rank %d]  source %d / data %d (%d)\n", rank, status.MPI_SOURCE, cost, status.MPI_TAG);
        if(status.MPI_TAG == COLOR_TAG) {
            if(rank == taskNum - 1 )
                MPI_Isend( &color, 1, MPI_INT, rank + 1, COLOR_TAG, SURVIVAL_COMM, &request);
            else MPI_Isend( &color, 1, MPI_INT, 0, TERMINATE_TAG, SURVIVAL_COMM, &request);
            // break;
        }
        if(status.MPI_TAG == TERMINATE_TAG) {
            if(rank != taskNum - 1 )
                MPI_Isend( &color, 1, MPI_INT, rank + 1, TERMINATE_TAG, SURVIVAL_COMM, &request);
            break;
        }
    }



    




    MPI_Finalize();
    return 0;
}