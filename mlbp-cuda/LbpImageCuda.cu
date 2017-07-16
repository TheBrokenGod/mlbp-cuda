#include "LbpImageCuda.h"
#include "cuda.h"
#include "cuda_runtime.h"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <cstdint>
#include <exception>

LbpImageCuda::LbpImageCuda(const std::vector<byte>& pixels, unsigned width, unsigned height) :
	AbstractLbpImage(pixels, width, height),
	d_pixels(nullptr),
	d_offsets(nullptr)
{
	// Move grayscale image to device
	cudaMalloc((void**)&d_pixels, width * height);
	cudaMemcpy((void*)d_pixels, (void*)this->pixels.data(), width * height, cudaMemcpyHostToDevice);
	this->pixels.clear();
}

LbpImageCuda::~LbpImageCuda() {
	cudaFree((void*)d_pixels);
}

__device__ bool isThreadInBounds(unsigned remainder) {
	// If this is the last block
	if(blockIdx.x + 1 == gridDim.x)
	{
		// If this thread lies over the array
		if(remainder > 0 && threadIdx.x >= remainder) {
			return false;
		}
	}
	return true;
}

__device__ long getGlobalIndex() {
	return threadIdx.x + blockIdx.x * blockDim.x;
}

__global__ void writeZeroIntoHistograms(float *histograms, unsigned lastBlockRemainder)
{
	if(!isThreadInBounds(lastBlockRemainder)) {
		return;
	}
	histograms[getGlobalIndex()] = 0.f;
}

__device__ float *getHistogram(float *histograms, unsigned row, unsigned col, unsigned histogramLength, unsigned blockEdge)
{
	int_pair block;
	block.y = row / blockEdge;
	block.x = col / blockEdge;
	long offset = histogramLength * (block.y * gridDim.x + block.x);
	return (histograms + offset);
}

__device__ byte pixelAt(byte *pixels, int row, unsigned width, int col)
{
	return pixels[row * width + col];
}

__device__ unsigned compareWithNeighborhood(byte *pixels, int_pair gaps_pixels, unsigned width, unsigned row, unsigned col, unsigned samples, int_pair *offsets)
{
	byte pixel = pixelAt(pixels, row, width, col);
	unsigned result = 0;

	for(int i = 0; i < samples; i++)
	{
		unsigned nrow = row + offsets[i].y;
		unsigned ncol = col + offsets[i].x;
		byte neighbor = pixelAt(pixels, nrow, width, ncol);

		if(pixel < neighbor) {
			int shift = samples - 1 - i;
			result = result | (0x1 << shift);
		}
	}

	return result;
}

__global__ void computeLBPs(byte *pixels, int_pair gaps_pixels, unsigned width, float *histograms, unsigned histogramLength, unsigned numberHistograms, unsigned samples, unsigned blockEdge, int_pair *offsets)
{
	unsigned row = threadIdx.y + blockIdx.y * blockEdge;
	unsigned col = threadIdx.x + blockIdx.x * blockEdge;
	unsigned imageRow = row + gaps_pixels.y;
	unsigned imageCol = col + gaps_pixels.x;

	unsigned pattern = compareWithNeighborhood(pixels, gaps_pixels, width, imageRow, imageCol, samples, offsets);
	float *histogram = getHistogram(histograms, row, col, histogramLength, blockEdge);
	atomicAdd(&histogram[pattern], 1.f);
}

__global__ void normalizeHistograms(float *histograms, float divider, unsigned lastBlockRemainder)
{
	if(!isThreadInBounds(lastBlockRemainder)) {
		return;
	}
	histograms[getGlobalIndex()] /= divider;
}

float *LbpImageCuda::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge)
{
	prepare(radius, samples, blockEdge);
	cudaMalloc((void**)&d_offsets, sizeof(int_pair) * offsets.size());
	cudaMemcpy((void*)d_offsets, (void*)offsets.data(), sizeof(int_pair) * offsets.size(), cudaMemcpyHostToDevice);

	// Allocate device memory
	float *d_histograms;
	cudaError_t error = cudaMalloc((void**)&d_histograms, getHistogramsSizeInBytes());
	if(error) {
		std::cerr << "cudaMalloc of " << getHistogramsSizeInBytes() << " bytes has failed" << std::endl;
		throw std::bad_alloc();
	}

	// Calculate CUDA grids
	dim3 lbpGridSize, lbpBlockSize;
	dim3 histGridSize, histBlockSize;
	unsigned remainder;
	calcLbpGridAndBlockSize(lbpGridSize, lbpBlockSize);
	calcHistGridAndBlockSize(histGridSize, histBlockSize, remainder);

	// Compute histrograms
	writeZeroIntoHistograms<<< histGridSize, histBlockSize >>>(
		d_histograms,
		remainder
	);
	computeLBPs<<< lbpGridSize, lbpBlockSize >>>(
		d_pixels,
		region.gaps_pixel,
		width,
		d_histograms,
		getHistogramLength(),
		getNumberHistograms(),
		samples,
		blockEdge,
		d_offsets
	);
	normalizeHistograms<<< histGridSize, histBlockSize >>>(
		d_histograms,
		blockEdge * blockEdge,
		remainder
	);

	// Copy result and return
	cudaMemcpy(histograms, d_histograms, getHistogramsSizeInBytes(), cudaMemcpyDeviceToHost);
	cudaFree((void*)d_offsets);
	cudaFree((void*)d_histograms);
	return histograms;
}

void LbpImageCuda::calcHistGridAndBlockSize(dim3& grid, dim3& block, unsigned& remainder)
{
	int device;
	cudaGetDevice(&device);
	struct cudaDeviceProp props;
	cudaGetDeviceProperties(&props, device);

	// Histograms will be covered by maximum-sized blocks
	auto numFloats = getHistogramsSizeInBytes() / sizeof(float);
	auto numBlocks = numFloats / props.maxThreadsPerBlock;
	remainder = numFloats % props.maxThreadsPerBlock;
	if(remainder > 0) {
		numBlocks++;
	}

	// Ensume maximum grid size is respected
	if(numBlocks > props.maxGridSize[0]) {
		std::cerr << "Histograms are too big" << std::endl;
		throw std::bad_alloc();
	}
	grid = {(unsigned)numBlocks};
	block = {(unsigned)props.maxThreadsPerBlock};
}

void LbpImageCuda::calcLbpGridAndBlockSize(dim3& grid, dim3& block)
{
	int device;
	cudaGetDevice(&device);
	struct cudaDeviceProp props;
	cudaGetDeviceProperties(&props, device);

	// Ensure maximum sizes are respected
	if(blockEdge * blockEdge > props.maxThreadsPerBlock) {
		std::cerr << "Maximum block edge on this device is " << std::sqrt(props.maxThreadsPerBlock) << std::endl;
		throw std::invalid_argument("");
	}
	if(region.grid_size.x > props.maxGridSize[0] || region.grid_size.y > props.maxGridSize[1]) {
		std::cerr << "Maximum grid size on this device is " << props.maxGridSize[0] << "x" << props.maxGridSize[1] << std::endl;
		throw std::invalid_argument("");
	}

	// Same as CPU implementation
	grid = {(unsigned)region.grid_size.x, (unsigned)region.grid_size.y};
	block = {blockEdge, blockEdge};
}
