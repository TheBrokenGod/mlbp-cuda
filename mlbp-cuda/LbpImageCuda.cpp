#include "LbpImageCuda.h"
#include "cuda.h"
#include "cuda_runtime.h"

LbpImageCuda::LbpImageCuda(const std::vector<byte>& pixels, unsigned width, unsigned height) :
	AbstractLbpImage(pixels, width, height),
	d_Pixels(nullptr)
{
	// Move grayscale image to device
	cudaMalloc((void**)d_Pixels, width * height);
	cudaMemcpy((void*)d_Pixels, (void*)this->pixels.data(), width * height, cudaMemcpyHostToDevice);
	this->pixels.clear();
}

LbpImageCuda::~LbpImageCuda() {
	cudaFree((void*)d_Pixels);
}

float *LbpImageCuda::calculateNormalizedLBPs()
{
	return nullptr;
}
