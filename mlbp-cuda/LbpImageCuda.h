#ifndef LBPIMAGECUDA_H_
#define LBPIMAGECUDA_H_

#include "AbstractLbpImage.h"
#include "cuda.h"

class LbpImageCuda : public AbstractLbpImage
{
public:
	/**
	 * Build a new LbpImageCuda object
	 *
	 * The computation will be done on the default CUDA stream.
	 */
	LbpImageCuda(const std::vector<byte>& pixels, unsigned width, unsigned height);
	/**
	 * See AbstractLbpImage::calculateNormalizedLBPs
	 */
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge);
	~LbpImageCuda();

private:
	byte *d_pixels;
	int_pair *d_offsets;
	void calcHistGridAndBlockSize(dim3& grid, dim3& block, unsigned& remainder);
	void calcLbpGridAndBlockSize(dim3& grid, dim3& block);
	struct cudaDeviceProp getDeviceProps();
};

#endif
