#ifndef LBPIMAGECUDA_H_
#define LBPIMAGECUDA_H_

#include "AbstractLbpImage.h"

class LbpImageCuda: public AbstractLbpImage
{
public:
	LbpImageCuda(const std::vector<byte>& pixels, unsigned width, unsigned height);
	~LbpImageCuda();
	float *calculateNormalizedLBPs();

private:
	byte *d_Pixels;
};

#endif
