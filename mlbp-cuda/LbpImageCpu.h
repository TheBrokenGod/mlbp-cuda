#ifndef LBPIMAGECPU_H_
#define LBPIMAGECPU_H_

#include "AbstractLbpImage.h"

class LbpImageCpu : public AbstractLbpImage
{
public:
	LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height);
	~LbpImageCpu();
	float *calculateNormalizedLBPs(float radius, unsigned blockEdge);
	float *calculateNormalizedLBPs(float radius, unsigned blockEdge, std::string visualOutput);

private:
	std::vector<byte> pixels;
	byte pixelAt(unsigned row, unsigned col);
	unsigned compareWithNeighborhood(unsigned row, unsigned col);
	void toGrayscale();
};

#endif
