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
	void prepare(float radius, unsigned samples, unsigned blockEdge);
	virtual float *calculateNormalizedLBPs() = 0;

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
	AbstractLbpImage(const std::vector<byte>& rgbPixels, unsigned width, unsigned height);
	virtual std::vector<byte> toGrayscale(const std::vector<byte>& rgbaPixels);

private:
	bool checkMinimumSize();
	void calcSamplingOffsets();
	void calcImageRegion();
};

#endif
