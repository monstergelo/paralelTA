
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

__global__ void addKernel(int *a)
{
	int i = blockIdx.x * blockDim.x + threadIdx.x;
	a[i] = i ^ 2;
}

extern "C" int
BurdenGPU()
{
	const int arraySize = 5000000;
	const int *a;
	int *GPU_a;

	a = (int*) malloc(arraySize*sizeof(int));
	cudaMallocManaged(&GPU_a, arraySize*sizeof(int));

	int NumBlock = 50;
	int blockSize = arraySize / NumBlock;

	addKernel <<< NumBlock, blockSize >>>(GPU_a);
	cudaDeviceSynchronize();

	cudaMemcpy(GPU_a, a, arraySize * sizeof(int), cudaMemcpyDeviceToHost);


	return 0;
}

