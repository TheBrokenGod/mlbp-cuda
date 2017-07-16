#ifndef ABSTRACTLBPIMAGE_H_
#define ABSTRACTLBPIMAGE_H_

#include <vector>
#include <cmath>
#include <string>
#include "Types.h"

class AbstractLbpImage
{
public:
	virtual ~AbstractLbpImage();
	virtual float *calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge) = 0;
	long getHistogramLength();
	long getNumberHistograms();

protected:
	std::vector<byte> pixels;
	unsigned width, height;
	float radius;
	unsigned samples;
	unsigned blockEdge;
	image_region region;
	std::vector<int_pair> offsets;
	float *histograms;

protected:
	AbstractLbpImage(const std::vector<byte>& pixels, unsigned width, unsigned height);
	void prepare(float radius, unsigned samples, unsigned blockEdge);
	void allocateHistograms();
	long getHistogramsSizeInBytes();

private:
	bool checkMinimumSize();
	void calcSamplingOffsets();
	void calcImageRegion();
	static std::vector<byte> toGrayscale(const std::vector<byte>& pixels, unsigned width, unsigned height);
};

#endif
