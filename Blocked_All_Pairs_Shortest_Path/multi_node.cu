#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <mpi.h>

#define INFINITE 1000000000
int* distance_host;
int* distance_dev;


__global__ void FW1(int *distance_dev, size_t r, int vertexPadded){

    size_t i = r * blockDim.x + threadIdx.y;
    size_t j = r * blockDim.x + threadIdx.x;
    size_t offset = i * vertexPadded;

    extern __shared__ int dist[];
    dist[threadIdx.y * blockDim.x + threadIdx.x] = distance_dev[ offset + j ];
    __syncthreads();

    for(size_t k=0; k<blockDim.x; ++k){
        dist[ threadIdx.y * blockDim.x + threadIdx.x ] = min(dist[ threadIdx.y * blockDim.x + threadIdx.x ], dist[ threadIdx.y * blockDim.x + k ] + dist[ k * blockDim.x + threadIdx.x ]);
        __syncthreads();
    }
    distance_dev[ offset + j ] = dist[threadIdx.y * blockDim.x + threadIdx.x];
}


__global__ void FW2(int *distance_dev, size_t r, int vertexPadded, size_t total_round){

    size_t block_i, block_j;
    if(blockIdx.y == 0){
        block_i = r;
        block_j = (blockIdx.x + r + 1) % total_round;
    }else{
        block_j = r;
        block_i = (blockIdx.x + r + 1) % total_round;
    }

    size_t i = block_i * blockDim.x + threadIdx.y;
    size_t j = block_j * blockDim.x + threadIdx.x;
    size_t offset = i * vertexPadded;
    size_t index = threadIdx.y * blockDim.x + threadIdx.x;
    size_t blockSize_squard = blockDim.x * blockDim.x;


    extern __shared__ int dist[];
    dist[index] = distance_dev[offset + j];
    dist[blockSize_squard + index] = distance_dev[offset + threadIdx.x + r * blockDim.x];
    dist[2*blockSize_squard + index] = distance_dev[(threadIdx.y + r * blockDim.x) * vertexPadded + j ];
    __syncthreads();

    
    for (size_t k = 0; k < blockDim.x; k++) {
        size_t ik = threadIdx.y * blockDim.x + blockSize_squard + k;
        size_t kj = k * blockDim.x + 2 * blockSize_squard + threadIdx.x;
        dist[index] = min(dist[index], dist[ik] + dist[kj]);
        __syncthreads();
    }
    distance_dev[offset + j] = dist[index];
}


__global__ void FW3(int *distance_dev, size_t r, int vertexPadded, size_t total_round, int BlockOffset, int id){

    size_t block_i = blockIdx.y + BlockOffset;
    size_t block_j = (r + blockIdx.x + 1) % total_round;
    size_t i = block_i * blockDim.x + threadIdx.y;
    size_t j = block_j * blockDim.x + threadIdx.x;
    size_t offset = i * vertexPadded;

    size_t index = threadIdx.y * blockDim.x + threadIdx.x;
    size_t blockSize_squard = blockDim.x * blockDim.x;

    extern __shared__ int dist[];
    dist[index] = distance_dev[offset + j]; //block(i,j)
    dist[blockSize_squard + index] = distance_dev[offset + threadIdx.x + r * blockDim.x]; //block(i,r)
    dist[2*blockSize_squard + index] = distance_dev[(threadIdx.y + r * blockDim.x) * vertexPadded + j ]; //block(r,j)
    __syncthreads();

    
    for(size_t k = 0; k < blockDim.x; k++) {
        size_t ik = threadIdx.y * blockDim.x + blockSize_squard + k;
        size_t kj = k * blockDim.x + 2 * blockSize_squard + threadIdx.x;
        dist[index] = min(dist[index], dist[ik] + dist[kj]);
        __syncthreads();
    }
    distance_dev[offset + j] = dist[index];
}



int main(int argc, char **argv){

	int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //get number of threads per block
    // int deviceNum;
    cudaSetDevice(rank);
    // cudaGetDeviceCount(&deviceNum);
    // cudaDeviceProp prop;
    // cudaGetDeviceProperties(&prop, 0);
    // int ThreadsPerBlock = (int) sqrt(prop.maxThreadsPerBlock);
    // int blockSize = ThreadsPerBlock;
    int deviceNum = 2;
    int blockSize = 32;
    
    
    //read input file
    std::ifstream inputFile(argv[1], std::ios::in | std::ios::binary);
    unsigned vertexNum, edgeNum;
    inputFile.read((char*)&vertexNum, 4);
    inputFile.read((char*)&edgeNum, 4);

    //calculate block number, vertex number in a block
    if(vertexNum < blockSize) blockSize = vertexNum;
    int blockNum = ceil( 1.0 * vertexNum / blockSize);
    int vertexPadded = blockSize * blockNum;


    //Allocate memory (pinned)
    cudaMallocHost(&distance_host, sizeof(int) * vertexPadded * vertexPadded);

    for(unsigned i=0; i<vertexPadded; ++i){
        for(unsigned j=0; j<vertexPadded; ++j){
			if( i!=j || i>=vertexNum || j>=vertexNum) distance_host[ i * vertexPadded + j ] = INFINITE;
            else distance_host[ i * vertexPadded + j ] = 0;
        }
    }

    int source, destination, weight;
    while( inputFile.read((char*)&source, 4) ){
        inputFile.read((char*)&destination, 4);
        inputFile.read((char*)&weight, 4);
        distance_host[ source * vertexPadded + destination ] = weight;
    }
    inputFile.close();

    
    //Blocked APSP
	size_t task_BlockNum, task_rBegin;
	size_t round = vertexPadded / blockSize;
	if(!rank){
		task_rBegin = round / deviceNum;
		task_BlockNum = task_rBegin + round % deviceNum;
	}else{
		task_rBegin = 0;
		task_BlockNum = round/deviceNum;
	}
	size_t task_rEnd = task_rBegin + task_BlockNum;
	size_t offset = task_rBegin * blockSize * vertexPadded;

    int blockRowSize = vertexPadded * blockSize;
	dim3 block(blockSize, blockSize);
    dim3 grid2(round-1, 2);
	dim3 grid3(round-1, task_BlockNum);
	
	cudaMalloc(&distance_dev, sizeof(int) * vertexPadded * vertexPadded);
    cudaMemcpy(&distance_dev[offset], &distance_host[offset], sizeof(int) * blockRowSize * task_BlockNum, cudaMemcpyHostToDevice);

	for (size_t r = 0; r < round; ++r) {

		size_t index = r * blockSize * vertexPadded;
		if(r >= task_rBegin && r < task_rEnd){
			cudaMemcpy(&distance_host[index], &distance_dev[index], sizeof(int) * blockRowSize, cudaMemcpyDeviceToHost);
			MPI_Send(&distance_host[index], blockRowSize, MPI_INT, !rank, 0, MPI_COMM_WORLD);
		}else{
			MPI_Recv(&distance_host[index], blockRowSize, MPI_INT, !rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			cudaMemcpy(&distance_dev[index], &distance_host[index], sizeof(int) * blockRowSize, cudaMemcpyHostToDevice);
		}

		FW1<<< 1, block, blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded);
		FW2<<< grid2, block, 3 * blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded, round);
		FW3<<< grid3, block, 3 * blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded, round, task_rBegin, rank);
		// cudaDeviceSynchronize();
	}
	cudaMemcpy( &distance_host[offset], &distance_dev[offset], sizeof(int) * blockRowSize * task_BlockNum, cudaMemcpyDeviceToHost);


	if(rank) MPI_Send(&distance_host[0], blockRowSize * task_BlockNum, MPI_INT, 0, 0, MPI_COMM_WORLD);
	else{
		MPI_Recv(&distance_host[0], blockRowSize * round/deviceNum, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		//write answer to output file
		std::ofstream outputFile(argv[2], std::ios::out | std::ios::binary);
		for(int i=0; i<vertexNum; ++i){
			for(int j=0; j<vertexNum; ++j){
				outputFile.write( (char*)&distance_host[ i * vertexPadded + j ], 4);
			}
		}
		outputFile.close();
	}
    
    cudaFree(distance_host);
	cudaFree(distance_dev);
	
	MPI_Finalize(); 
    return 0;
}