#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>
#include "lodepng.h"

LbpImageCpu::LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height) :
	AbstractLbpImage(pixels, width, height) {
}

LbpImageCpu::~LbpImageCpu() {
}

byte LbpImageCpu::pixelAt(unsigned row, unsigned col)
{
	return pixels[row * width + col];
}

float *LbpImageCpu::getHistogram(float *histograms, unsigned row, unsigned col)
{
	// Each pixel block has its own histogram
	int_pair block;
	block.y = (row - region.gaps_pixel.y) / blockEdge;
	block.x = (col - region.gaps_pixel.x) / blockEdge;
	auto offset = getHistogramLength() * (block.y * region.grid_size.x + block.x);
	return (histograms + offset);
}

unsigned LbpImageCpu::compareWithNeighborhood(unsigned row, unsigned col)
{
	byte pixel = pixelAt(row, col);
	unsigned result = 0;

	// Compare with each neighbor
	for(int i = 0; i < samples; i++)
	{
		unsigned nrow = row + offsets[i].y;
		unsigned ncol = col + offsets[i].x;
		byte neighbor = pixelAt(nrow, ncol);

		// Write one if LE
		if(pixel <= neighbor) {
			int shift = samples - 1 - i;
			result = result | (0x1 << shift);
		}
	}

	return result;
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, samples, blockEdge, "");
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge, const std::string& outputImageName)
{
	prepare(radius, samples, blockEdge);
	// Reserve for image output
	std::vector<byte> output;
	unsigned outputWidth;
	unsigned outputHeight;
	if(outputImageName.size() > 0) {
		outputWidth = region.grid_size.x * blockEdge;
		outputHeight = region.grid_size.y * blockEdge;
		output.reserve(outputWidth * outputHeight);
	}

	// Calculate for net area
	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			// Get pointer to block's histogram
			float *histogram = getHistogram(histograms, i, j);
			// Pixel's LBP will become array index
			unsigned pattern = compareWithNeighborhood(i, j);
			// Increment entry
			histogram[pattern] += 1;

			if(outputImageName.size() > 0) {
				float check = pattern / powf(2.f, samples - 8.f);
				output.push_back((byte)roundf(check));
			}
		}
	}

	// Normalize to 1
	int size = getHistogramLength() * getNumberHistograms();
	for(int i = 0; i < size; i++)
	{
		histograms[i] /= (blockEdge * blockEdge);
	}

	// Save check image
	if(outputImageName.size() > 0) {
		lodepng::encode(outputImageName + ".png", output, outputWidth, outputHeight, LCT_GREY);
	}

	return histograms;
}
