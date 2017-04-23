#ifndef ABSTRACTLBPIMAGE_H_
#define ABSTRACTLBPIMAGE_H_

#include <vector>
#include <cmath>
#include "Types.h"

class AbstractLbpImage
{
public:
	virtual ~AbstractLbpImage();
	bool checkMinimumSize(float radius, unsigned blockEdge);
	void calcSamplingOffsets(float radius, unsigned samples);
	void calcImageRegion(float radius, unsigned blockEdge);
	virtual float *calculateNormalizedLBPs(float radius, unsigned blockEdge) = 0;

protected:
	const std::vector<byte>& pixels;
	unsigned width, height;
	image_region region;
	std::vector<int_pair> offsets;
	AbstractLbpImage(const std::vector<byte>& pixels, unsigned width, unsigned height);

};

#endif
