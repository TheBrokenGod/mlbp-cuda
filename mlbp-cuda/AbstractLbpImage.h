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
	/**
	 * Perform the LBP computation
	 */
	virtual std::vector<float> calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge) = 0;
	/**
	 * Return the number of floats in one histogram
	 */
	long getHistogramLength() const;
	/**
	 * Return the dimension of the grid in x (columns) and y (rows) blocks
	 *
	 * Each block has its own histogram
	 */
	int_pair getGridSize() const;
	/**
	 * Return the number of histograms which is grid.x * grid.y
	 */
	long getNumberHistograms() const;
	/**
	 * Save the histograms to a text file; one histogram per row
	 */
	static void saveHistogramsToFile(std::vector<float> histograms, long histLength, long numHists, const std::string& filename);

protected:
	std::vector<byte> pixels;
	unsigned width, height;
	float radius;
	unsigned samples;
	unsigned blockEdge;
	image_region region;
	std::vector<int_pair> offsets;
	std::vector<float> histograms;

protected:
	AbstractLbpImage(const std::vector<byte>& pixels, unsigned width, unsigned height);
	void prepare(float radius, unsigned samples, unsigned blockEdge);
	long getHistogramsSizeInBytes();

private:
	static std::vector<byte> toGrayscale(const std::vector<byte>& pixels, unsigned width, unsigned height);
	bool checkMinimumSize();
	void calcSamplingOffsets();
	void calcImageRegion();
	void allocateHistograms();
};

#endif
