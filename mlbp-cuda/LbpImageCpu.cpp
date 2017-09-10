#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>

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

std::vector<float> LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, samples, blockEdge, nullptr, nullptr, nullptr);
}

std::vector<float> LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge, std::vector<byte> *outputImage, unsigned *outputWidth, unsigned *outputHeight)
{
	prepare(radius, samples, blockEdge);
	if(outputImage != nullptr) {
		outputImage->clear();
		*outputWidth = region.grid_size.x * blockEdge;
		*outputHeight = region.grid_size.y * blockEdge;
		outputImage->reserve(*outputWidth * *outputHeight);
	}

	// Calculate for net area
	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			// Get pointer to block's histogram
			float *histogram = getHistogram(histograms.data(), i, j);
			// Pixel's LBP will become array index
			unsigned pattern = compareWithNeighborhood(i, j);
			// Increment entry
			histogram[pattern] += 1;
			// Save representation
			if(outputImage != nullptr) {
				float check = pattern / powf(2.f, samples - 8.f);
				outputImage->push_back((byte)roundf(check));
			}
		}
	}
	// Normalize to one
	int size = getHistogramLength() * getNumberHistograms();
	for(int i = 0; i < size; i++)
	{
		histograms[i] /= (blockEdge * blockEdge);
	}
	// Free internal memory
	return std::move(histograms);
}
