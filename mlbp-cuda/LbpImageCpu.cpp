#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iomanip>
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

		// Write one if lower
		if(pixel < neighbor) {
			int shift = samples - 1 - i;
			result = result | (0x1 << shift);
		}
	}

	return result;
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

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, samples, blockEdge, "");
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned samples, unsigned blockEdge, const std::string& outputFileName)
{
	prepare(radius, samples, blockEdge);
	// Reserve for visual output
	std::vector<byte> output;
	unsigned outputWidth;
	unsigned outputHeight;
	if(outputFileName.size() > 0) {
		outputWidth = region.grid_size.x * blockEdge;
		outputHeight = region.grid_size.y * blockEdge;
		output.reserve(outputWidth * outputHeight);
	}

	// Calculate histograms
	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			float *histogram = getHistogram(histograms, i, j);
			unsigned pattern = compareWithNeighborhood(i, j);

			if(outputFileName.size() > 0) {
				float check = pattern / powf(2.f, samples - 8.f);
				output.push_back((byte)roundf(check));
			}

			int temp1 = getHistogramLength();
			int temp2 = getNumberHistograms();
			long temp3 = getHistogramsSizeInBytes();
			histogram[pattern] += 1;
		}
	}

	// Normalize
	int size = getHistogramLength() * getNumberHistograms();
	for(int i = 0; i < size; i++)
	{
		histograms[i] /= (blockEdge * blockEdge);
	}

	if(outputFileName.size() > 0) {
		// Save check image
		lodepng::encode(outputFileName + ".png", output, outputWidth, outputHeight, LCT_GREY);

		// Save histograms to text file
		std::ofstream file(outputFileName + ".lbp");
		file << std::fixed << std::setprecision(4);
		for(int i = 0; i < getNumberHistograms(); i++)
		{
			float *histogram = (histograms + i * getHistogramLength());
			for(int j = 0; j < getHistogramLength(); j++)
			{
				file << histogram[j]  << ' ';
			}
			file << '\n';
		}
		file.close();
	}
	return histograms;
}
