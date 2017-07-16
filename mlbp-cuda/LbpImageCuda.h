#ifndef LBPIMAGECUDA_H_
#define LBPIMAGECUDA_H_

#include "AbstractLbpImage.h"

class LbpImageCuda: public AbstractLbpImage
{
public:
	LbpImageCuda(const std::vector<byte>& pixels, unsigned width, unsigned height);
	~LbpImageCuda();
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge);

private:
	byte *d_pixels;
	int_pair *d_offsets;
	void calcHistDim(dim3& grid, dim3& block, unsigned& remainder);
};

#endif
