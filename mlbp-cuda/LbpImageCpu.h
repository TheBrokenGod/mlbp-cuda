#ifndef LBPIMAGECPU_H_
#define LBPIMAGECPU_H_

#include "AbstractLbpImage.h"

class LbpImageCpu : public AbstractLbpImage
{
public:
	/**
	 * Build a new LbpImageCpu object
	 *
	 * The computation will be done on the CPU.
	 */
	LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height);
	/**
	 * Perform the LBP computation
	 *
	 * An image representing the local binary patterns will be saved
	 */
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge, const std::string& outputImageName);
	/**
	 * See AbstractLbpImage::calculateNormalizedLBPs
	 */
	float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge);
	~LbpImageCpu();

private:
	byte pixelAt(unsigned row, unsigned col);
	float *getHistogram(float *histograms, unsigned row, unsigned col);
	unsigned compareWithNeighborhood(unsigned row, unsigned col);
};

#endif
