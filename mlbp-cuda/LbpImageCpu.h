#ifndef LBPIMAGECPU_H_
#define LBPIMAGECPU_H_

#include "AbstractLbpImage.h"

class LbpImageCpu : public AbstractLbpImage
{
public:
	~LbpImageCpu();
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
	std::vector<float> calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge,
			std::vector<byte> *outputImage, unsigned *outputWidth, unsigned *outputHeight);
	/**
	 * See AbstractLbpImage::calculateNormalizedLBPs
	 */
	std::vector<float> calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge);

private:
	byte pixelAt(unsigned row, unsigned col);
	float *getHistogram(float *histograms, unsigned row, unsigned col);
	unsigned compareWithNeighborhood(unsigned row, unsigned col);
};

#endif
