#include<iostream>
#include<cstdlib>   //for atoi
#include<iomanip>   //for set precision
#include<algorithm> //for sort, swap
#include<cmath>		//for ceil
#include"mpi.h"

float *result;

void mergeEnd(char &change, int &mySize, float *&myBuf, float *&otherBuf, int &otherSize){
	int myIndex = mySize - 1;
	int otherIndex = otherSize - 1;
	//--otherSize;
	for(int i = mySize-1; i >= 0; --i){
		if(  (otherIndex >= 0) && *(myBuf + myIndex) >= *(otherBuf + otherIndex) ){
			*(result + i) = *(myBuf + myIndex);
			--myIndex;
			
		}else if( (otherIndex >= 0) && *(myBuf + myIndex) < *(otherBuf + otherIndex) ) {
			*(result + i) = *(otherBuf + otherIndex);
			--otherIndex;
			change = 1;
		}else{
			*(result + i) = *(myBuf + myIndex);
			--myIndex;
		}
	}
	std::swap(myBuf, result);
}

void mergeFront(char &change, int &mySize, float *&myBuf, float *&otherBuf, int &otherSize){
	int myIndex = 0, otherIndex = 0;
	for(int i = 0; i < mySize; ++i){

		if(  (otherIndex <= otherSize-1 ) && *(myBuf + myIndex) <= *(otherBuf + otherIndex) ){
			*(result + i) = *(myBuf + myIndex);
			++myIndex;
		}else if(  (otherIndex <= otherSize-1 ) && *(myBuf + myIndex) > *(otherBuf + otherIndex) ){
			*(result + i) = *(otherBuf + otherIndex);
			++otherIndex;
			change = 1;
		}else{
			*(result + i) = *(myBuf + myIndex);
			++myIndex;
		}
	}	
	std::swap(myBuf, result);
}

int main(int argc, char *argv[]){
	
	int rc, taskNum, rank;
	int globalProblemSize = atoi(argv[1]);
	int taskProblemSize, preTaskProblemSize, nextTaskProblemSize;
	
	char* inputFileName = argv[2];
	char* outputFileName = argv[3];
	MPI_File inputFile, outputFile;

	MPI_Comm SURVIVAL_COMM;
	MPI_Group WORLD_GROUP, SURVIVAL_GROUP;

	
	//check the number of parameter
	//If it wouldn't meet the require, show error message and terminate the program.
	// if(argc != 4){
	// 	std::cout<<"Error number of parameter. Terminating."<<std::endl;
	// 	return 0;
	// }
	
	//initialize the MPI execution environment
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_Init(&argc, &argv);
	// if(rc != MPI_SUCCESS){
	// 	std::cout<<"Error starting MPI program. Terminating."<<std::endl;
	// 	MPI_Abort(MPI_COMM_WORLD, rc);
	// }
	
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
	if(rank < remainder){
		taskProblemSize = quotient + 1;	
		begin += rank;
	}else{
		taskProblemSize = quotient;	
		begin += remainder;
	}
	int otherSize = ceil(ceil(1.0*quotient)/2.0);
	if(otherSize >= quotient) otherSize = quotient;

 	//open a file with readonly mode
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_File_open(SURVIVAL_COMM, inputFileName, MPI_MODE_RDONLY, MPI_INFO_NULL, &inputFile);
	// if(rc != MPI_SUCCESS){
	// 	std::cout<<"Error opening the file. Terminating."<<std::endl;
	// 	MPI_Abort(SURVIVAL_COMM, rc);
	// }
	
	//read a file
	//If it wouldn't success, show the error message and abort it.
	result = new float[taskProblemSize];
	float *buf = new float[taskProblemSize];
	int offset = begin * sizeof(MPI_FLOAT);
	rc = MPI_File_read_at(inputFile, offset, buf, taskProblemSize, MPI_FLOAT, MPI_STATUS_IGNORE);
	// if(rc != MPI_SUCCESS){
	// 	std::cout<<"Error reading the file. Terminating."<<std::endl;
	// 	MPI_Abort(SURVIVAL_COMM, rc);
	// }
	
	//close input file
	//If it wouldn't success, show the error message and abort it.
	rc = MPI_File_close(&inputFile);
	// if(rc != MPI_SUCCESS){
	// 	std::cout<<"Error closing the file. Terminating."<<std::endl;
	// 	MPI_Abort(SURVIVAL_COMM, rc);
	// }

	//sort buf of each task with ascending order
	std::sort(buf, buf+taskProblemSize);

	/********** odd even sort ***********/
	int pre_rank = rank - 1;
	int next_rank = rank + 1;
	
	float *recvBuf = new float[otherSize];
	char change = 0, globalChange = 1;
	while(globalChange){
		globalChange = 0;
		change = 0;
		// odd phase
		// rank is odd and it is not the last task
		if( (rank & 1) && (next_rank != taskNum) ){
			// send(receive) a buf to(from) next task
			MPI_Sendrecv( buf+taskProblemSize-otherSize, otherSize, MPI_FLOAT, next_rank, 1, recvBuf, otherSize, MPI_FLOAT, next_rank, 0, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			mergeFront(change, taskProblemSize, buf, recvBuf, otherSize);
			// rank is even and it is not the first task
		}else if( !(rank & 1) && (rank!=0) ){
			// send(receive) a buf to(from) previous task
			MPI_Sendrecv( buf, otherSize, MPI_FLOAT, pre_rank, 0, recvBuf, otherSize, MPI_FLOAT, pre_rank, 1, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			mergeEnd(change, taskProblemSize, buf, recvBuf, otherSize);
		}
		// even phase
		// rank is odd -> must send buf to previous task
		if( rank & 1 ){
			// send(receive) a buf to(from) previous task
			MPI_Sendrecv( buf, otherSize, MPI_FLOAT, pre_rank, 0, recvBuf, otherSize, MPI_FLOAT, pre_rank, 1, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			mergeEnd(change, taskProblemSize, buf, recvBuf, otherSize);
		}else if( next_rank != taskNum ){
			// send(receive) a buf to(from) next task
			MPI_Sendrecv( buf+taskProblemSize-otherSize, otherSize, MPI_FLOAT, next_rank, 1, recvBuf, otherSize, MPI_FLOAT, next_rank, 0, SURVIVAL_COMM, MPI_STATUS_IGNORE);
			mergeFront(change, taskProblemSize, buf, recvBuf, otherSize);
		}
		MPI_Allreduce(&change, &globalChange, 1, MPI_CHAR, MPI_BOR, SURVIVAL_COMM);
	}
	/********** odd even sort ***********/

	//write result into file
	MPI_File_open(SURVIVAL_COMM, outputFileName, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &outputFile);
	MPI_File_write_at(outputFile, offset, buf, taskProblemSize, MPI_FLOAT, MPI_STATUS_IGNORE);
	MPI_File_close(&outputFile);

	delete buf;
	delete result;
	delete recvBuf;
	MPI_Finalize();
	return 0;
}