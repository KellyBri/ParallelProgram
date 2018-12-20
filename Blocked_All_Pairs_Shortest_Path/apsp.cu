#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>

#define INFINITE 1000000000
int* distance_host;
int* distance_dev;


__global__ void FW1(int *distance_dev, int r, int vertexPadded){

    int i = r * blockDim.x + threadIdx.y;
    int j = r * blockDim.x + threadIdx.x;
    int offset = i * vertexPadded;

    extern __shared__ int dist[];
    dist[threadIdx.y * blockDim.x + threadIdx.x] = distance_dev[ offset + j ];
    __syncthreads();

    for(int k=0; k<blockDim.x; ++k){
        dist[ threadIdx.y * blockDim.x + threadIdx.x ] = min(dist[ threadIdx.y * blockDim.x + threadIdx.x ], dist[ threadIdx.y * blockDim.x + k ] + dist[ k * blockDim.x + threadIdx.x ]);
        __syncthreads();
    }
    distance_dev[ offset + j ] = dist[threadIdx.y * blockDim.x + threadIdx.x];
}


__global__ void FW2(int *distance_dev, int r, int vertexPadded, int total_round){

    int block_i, block_j;
    if(blockIdx.y == 0){
        block_i = r;
        block_j = (blockIdx.x + r + 1) % total_round;
    }else{
        block_j = r;
        block_i = (blockIdx.x + r + 1) % total_round;
    }

    int i = block_i * blockDim.x + threadIdx.y;
    int j = block_j * blockDim.x + threadIdx.x;
    int offset = i * vertexPadded;
    int index = threadIdx.y * blockDim.x + threadIdx.x;
    int blockSize_squard = blockDim.x * blockDim.x;


    extern __shared__ int dist[];
    dist[index] = distance_dev[offset + j];
    dist[blockSize_squard + index] = distance_dev[offset + threadIdx.x + r * blockDim.x];
    dist[2*blockSize_squard + index] = distance_dev[(threadIdx.y + r * blockDim.x) * vertexPadded + j ];
    __syncthreads();

    
    for (int k = 0; k < blockDim.x; k++) {
        int ik = threadIdx.y * blockDim.x + blockSize_squard + k;
        int kj = k * blockDim.x + 2 * blockSize_squard + threadIdx.x;
        dist[index] = min(dist[index], dist[ik] + dist[kj]);
        __syncthreads();
    }
    distance_dev[offset + j] = dist[index];
}


__global__ void FW3(int *distance_dev, int r, int vertexPadded, int total_round){

    int block_i = (r + blockIdx.y + 1) % total_round;
    int block_j = (r + blockIdx.x + 1) % total_round;

    int i = block_i * blockDim.x + threadIdx.y;
    int j = block_j * blockDim.x + threadIdx.x;
    int offset = i * vertexPadded;

    int index = threadIdx.y * blockDim.x + threadIdx.x;
    int blockSize_squard = blockDim.x * blockDim.x;

    extern __shared__ int dist[];
    dist[index] = distance_dev[offset + j]; //block(i,j)
    dist[blockSize_squard + index] = distance_dev[offset + threadIdx.x + r * blockDim.x]; //block(i,r)
    dist[2*blockSize_squard + index] = distance_dev[(threadIdx.y + r * blockDim.x) * vertexPadded + j ]; //block(r,j)
    __syncthreads();

    
    for (int k = 0; k < blockDim.x; k++) {
        int ik = threadIdx.y * blockDim.x + blockSize_squard + k;
        int kj = k * blockDim.x + 2 * blockSize_squard + threadIdx.x;
        dist[index] = min(dist[index], dist[ik] + dist[kj]);
        __syncthreads();
    }
    distance_dev[offset + j] = dist[index];
}

void block_FW(int blockSize, int vertexNum, int vertexPadded) {
    int round = vertexPadded / blockSize;
    dim3 block(blockSize, blockSize);
    dim3 grid2(round-1, 2);
    dim3 grid3(round-1, round-1);
    cudaMalloc(&distance_dev, sizeof(int) * vertexPadded * vertexPadded);
    cudaMemcpy(distance_dev, distance_host, sizeof(int) * vertexPadded * vertexPadded, cudaMemcpyHostToDevice);

	for (int r = 0; r < round; ++r) {
        FW1<<< 1, block, blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded);
        FW2<<< grid2, block, 3 * blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded, round);
        FW3<<< grid3, block, 3 * blockSize * blockSize * sizeof(int) >>>(distance_dev, r, vertexPadded, round);
    }
    cudaMemcpy(distance_host, distance_dev, sizeof(int) * vertexPadded * vertexPadded, cudaMemcpyDeviceToHost);
}


int main(int argc, char **argv){

    //get number of threads per block
    cudaSetDevice(0);
    cudaDeviceProp prop;
    cudaGetDeviceProperties(&prop, 0);
    int ThreadsPerBlock = (int) sqrt(prop.maxThreadsPerBlock);
    int blockSize = ThreadsPerBlock;
    
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
            if( i>=vertexNum || j>=vertexNum) distance_host[ i * vertexPadded + j ] = INFINITE;
            else if( i == j) distance_host[ i * vertexPadded + j ] = 0;
            else distance_host[ i * vertexPadded + j ] = INFINITE;
        }
    }

    int source, destination, weight;
    while( inputFile.read((char*)&source, 4) ){
        inputFile.read((char*)&destination, 4);
        inputFile.read((char*)&weight, 4);
        distance_host[ source * vertexPadded + destination ] = weight;
    }
    inputFile.close();


    block_FW(blockSize, vertexNum, vertexPadded);

    
    //write answer to output file
    std::ofstream outputFile(argv[2], std::ios::out | std::ios::binary);
    for(int i=0; i<vertexNum; ++i){
        for(int j=0; j<vertexNum; ++j){
            outputFile.write( (char*)&distance_host[ i * vertexPadded + j ], 4);
        }
    }
    outputFile.close();
    
    cudaFree(distance_host);
    cudaFree(distance_dev);
    return 0;
}