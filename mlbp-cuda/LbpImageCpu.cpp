#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>
#include "lodepng.h"

LbpImageCpu::LbpImageCpu(const std::vector<byte>& rgbPixels, unsigned width, unsigned height) : AbstractLbpImage(rgbPixels, width, height),
	histograms(nullptr) {
}

LbpImageCpu::~LbpImageCpu() {
}

byte LbpImageCpu::pixelAt(unsigned row, unsigned col)
{
	return pixels[row * width + col];
}

unsigned LbpImageCpu::compareWithNeighborhood(unsigned row, unsigned col)
{
	byte pixel = pixelAt(row, col);
	unsigned result = 0;

	// Compare with each neighbor
	for(int i = 0; i < numSamples; i++)
	{
		unsigned nrow = row + offsets[i].y;
		unsigned ncol = col + offsets[i].x;
		byte neighbor = pixelAt(nrow, ncol);

		// Write one if lower
		if(pixel < neighbor) {
			int shift = numSamples - 1 - i;
			result = result | (0x1 << shift);
		}
	}

	return result;
}

void LbpImageCpu::allocateHistograms()
{
	long size = pow(2, numSamples) * region.grid_size.x * region.grid_size.y;
	try {
		histograms = new float[size];
		std::fill_n(histograms, size, 0.f);
	}
	catch(const std::bad_alloc& e) {
		std::cerr << "new float[" << size << "] allocation failed" << std::endl;
		throw e;
	}
}

float *LbpImageCpu::getHistogram(unsigned row, unsigned col, unsigned blockEdge)
{
	// Each pixel block has its histogram
	int_pair block;
	block.y = (row - region.gaps_pixel.y) / blockEdge;
	block.x = (col - region.gaps_pixel.x) / blockEdge;
	size_t offset = pow(2, numSamples) * (block.y * region.grid_size.x + block.x);
	return (histograms + offset);
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, blockEdge, "");
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge, std::string visualOutput)
{
	allocateHistograms();

	// Visual output if requested
	std::vector<byte> output;
	unsigned outputWidth;
	unsigned outputHeight;
	if(visualOutput.size() > 0) {
		outputWidth = region.grid_size.x * blockEdge;
		outputHeight = region.grid_size.y * blockEdge;
		output.reserve(outputWidth * outputHeight);
	}

	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			float *histogram = getHistogram(i, j, blockEdge);
			unsigned pattern = compareWithNeighborhood(i, j);

			if(visualOutput.size() > 0) {
				float check = pattern / powf(2.f, numSamples - 8.f);
				output.push_back((byte)roundf(check));
			}

			histogram[pattern] += 1;
		}
	}

	if(visualOutput.size() > 0) {
		lodepng::encode(visualOutput, output, outputWidth, outputHeight, LCT_GREY);
		std::cout << output.size() << " output pixels" << std::endl;
	}
	return histograms;
}
