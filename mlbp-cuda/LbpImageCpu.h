#ifndef LBPIMAGECPU_H_
#define LBPIMAGECPU_H_

#include "AbstractLbpImage.h"

class LbpImageCpu: public AbstractLbpImage
{
public:
	LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height);
	~LbpImageCpu();
	float *calculateNormalizedLBPs(float radius, unsigned blockEdge);

private:
	std::vector<byte> grayPixels;
	void toGrayscale();
	byte pixelAt(unsigned row, unsigned col);
	unsigned compareWithNeighborhood(unsigned row, unsigned col);
};

#endif
