#ifndef LBPIMAGECPU_H_
#define LBPIMAGECPU_H_

#include "AbstractLbpImage.h"

class LbpImageCpu : public AbstractLbpImage
{
public:
	LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height);
	~LbpImageCpu();
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge);
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge, const std::string& outputFilesName);

private:
	byte pixelAt(unsigned row, unsigned col);
	unsigned compareWithNeighborhood(unsigned row, unsigned col);
	float *getHistogram(unsigned row, unsigned col);
};

#endif
