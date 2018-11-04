#include<iostream>
#include<cstdlib>   //for atoi
#include<iomanip>   //for set precision
#include<algorithm> //for swap
#include"mpi.h"


int main(int argc, char *argv[]){
	
	int rc, taskNum, rank;
	int globalProblemSize = atoi(argv[1]);
	int taskProblemSize;
	
	char* inputFileName = argv[2];
	char* outputFileName = argv[3];
	MPI_File inputFile, outputFile;

	MPI_Comm SURVIVAL_COMM;
	MPI_Group WORLD_GROUP, SURVIVAL_GROUP;

	MPI_Status status;
	
	//check the number of parameter
	//If it wouldn't meet the require, show error message and terminate the program.
	if(argc != 4){
		std::cout<<"Error number of parameter. Terminating."<<std::endl;
		return 0;
	}
	
	//initialize the MPI execution environment
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_Init(&argc, &argv);
	if(rc != MPI_SUCCESS){
		std::cout<<"Error starting MPI program. Terminating."<<std::endl;
		MPI_Abort(MPI_COMM_WORLD, rc);
	}
	
	//determine the number of tasks in the group & the rank of each task within the communicator
	MPI_Comm_size(MPI_COMM_WORLD, &taskNum);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	SURVIVAL_COMM = MPI_COMM_WORLD;

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
	int begin = rank * quotient; //the first element of a rank
	taskProblemSize = quotient;
	if(rank < remainder){
		++taskProblemSize;
		begin += rank;
	}else{
		begin += remainder;
	}
	int end = begin + taskProblemSize - 1; //the last element of a rank

 	//open a file with readonly mode
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_File_open(SURVIVAL_COMM, inputFileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFile);
	if(rc != MPI_SUCCESS){
		std::cout<<"Error opening the file. Terminating."<<std::endl;
		MPI_Abort(SURVIVAL_COMM, rc);
	}
	
	//read a file
	//If it wouldn't success, show the error message and abort it.
	float *buf = new float[taskProblemSize];
	int offset = begin * sizeof(MPI_FLOAT);
	rc = MPI_File_read_at(inputFile, offset, buf, taskProblemSize, MPI_FLOAT, &status);
	if(rc != MPI_SUCCESS){
		std::cout<<"Error reading the file. Terminating."<<std::endl;
		MPI_Abort(SURVIVAL_COMM, rc);
	}
	
	//close input file
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_File_close(&inputFile);
	if(rc != MPI_SUCCESS){
		std::cout<<"Error closing the file. Terminating."<<std::endl;
		MPI_Abort(SURVIVAL_COMM, rc);
	}
	
	/********** odd even sort ***********/
	char globalChange = 1;
	while(globalChange){
		globalChange = 0;
		char taskChange = 0;
		/*  even phase  */
		//the index of first element is odd
		if(begin & 1){
			//sort 
			for(int i = 1; i < taskProblemSize - 1; i += 2){
				if( *(buf + i) > *(buf + i + 1) ){
					std::swap( *(buf + i), *(buf + i + 1) );
					taskChange = 1;
				}
			}
			//the index of first element is odd -> must need to send the first element to previous task
			//if this task is rank 0, no previous task to be sent
			if( rank != 0 ){
				float recvNum;
				//send(receive) a number to(from) previous task
				MPI_Sendrecv( buf, 1, MPI_FLOAT, rank-1, 0, &recvNum, 1, MPI_FLOAT, rank-1, 1, SURVIVAL_COMM, MPI_STATUS_IGNORE);
				if( *(buf) < recvNum ){
					*(buf) = recvNum;
					taskChange = 1;
				}
			}
		}
		//the index of first element is even
		else{
			//sort
			for(int i = 0; i < taskProblemSize; i += 2){
				if(i == (taskProblemSize - 1) ) break;
				if( *(buf + i) > *(buf + i + 1) ){
					std::swap( *(buf + i), *(buf + i + 1) );
					taskChange = 1;
				}
			}
		}
		//the index of last element is even -> must need to receive the last element from next task
		//if this task is not the last rank, no element can be received
		if( !(end & 1) && (rank != (taskNum-1) ) ){
			float recvNum;
			//send(receive) a number to(from) next task
			MPI_Sendrecv( buf+taskProblemSize-1, 1, MPI_FLOAT, rank+1, 1, &recvNum, 1, MPI_FLOAT, rank+1, 0, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			if( *(buf+taskProblemSize-1) > recvNum ){
				*(buf+taskProblemSize-1) = recvNum;
				taskChange = 1;
			}
		}

		//wait for all tasks to finish
		MPI_Barrier(SURVIVAL_COMM);
		
		
		/*  odd phase */	
		//the index of first element is odd
		if(begin & 1){
			//sort
			for(int i = 0; i < taskProblemSize - 1; i += 2){
				if( *(buf + i) > *(buf + i + 1) ){
					std::swap( *(buf + i), *(buf + i + 1) );
					taskChange = 1;
				}
			}
		}
		//the index of first element is even
		else{
			//sort 
			for(int i = 1; i < taskProblemSize - 1; i += 2){
				if( *(buf + i) > *(buf + i + 1) ){
					std::swap( *(buf + i), *(buf + i + 1) );
					taskChange = 1;
				}
			}
			//the index of first element is odd -> must need to send the first element to previous task
			//if this task is rank 0, no previous task to be sent
			if( rank != 0 ){
				float recvNum;
				//send(receive) a number to(from) next task
				MPI_Sendrecv( buf, 1, MPI_FLOAT, rank-1, 0, &recvNum, 1, MPI_FLOAT, rank-1, 1, SURVIVAL_COMM, MPI_STATUS_IGNORE);
				if( *(buf) < recvNum ){
					*(buf) = recvNum;
					taskChange = 1;
				}
			}
		}
		//the index of last element is odd -> must need to receive the last element from next task
		//if this task is not the last rank, no element can be received
		if( (end & 1) && (rank != taskNum-1) ){
			float recvNum;
			//send(receive) a number to(from) next task
			MPI_Sendrecv( buf+taskProblemSize-1, 1, MPI_FLOAT, rank+1, 1, &recvNum, 1, MPI_FLOAT, rank+1, 0, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			if( *(buf+taskProblemSize-1) > recvNum ){
				*(buf+taskProblemSize-1) = recvNum;
				taskChange = 1;
			}
		}
		MPI_Allreduce(&taskChange, &globalChange, 1, MPI_CHAR, MPI_BOR, SURVIVAL_COMM);
	}
	//write result into file
	MPI_File_open(SURVIVAL_COMM, outputFileName, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &outputFile);
	MPI_File_write_at(outputFile, offset, buf, taskProblemSize, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&outputFile);

	delete buf;
	MPI_Finalize();
	return 0;
}
