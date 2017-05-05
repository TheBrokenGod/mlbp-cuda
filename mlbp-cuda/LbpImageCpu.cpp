#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iomanip>
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

long LbpImageCpu::getHistogramLength() {
	return pow(2, numSamples);
}

long LbpImageCpu::getNumberHistograms()
{
	return region.grid_size.x * region.grid_size.y;
}

void LbpImageCpu::allocateHistograms()
{
	long size = getHistogramLength() * getNumberHistograms();
	try {
		histograms = new float[size];
	}
	catch(const std::bad_alloc& e) {
		std::cerr << "new float[" << size << "] allocation failed" << std::endl;
		throw e;
	}
	std::fill_n(histograms, size, 0.f);
}

float *LbpImageCpu::getHistogram(unsigned row, unsigned col, unsigned blockEdge)
{
	// Each pixel block has its own histogram
	int_pair block;
	block.y = (row - region.gaps_pixel.y) / blockEdge;
	block.x = (col - region.gaps_pixel.x) / blockEdge;
	auto offset = getHistogramLength() * (block.y * region.grid_size.x + block.x);
	return (histograms + offset);
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, blockEdge, "");
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge, std::string visualOutput)
{
	allocateHistograms();

	// Prepare visual output
	std::vector<byte> output;
	unsigned outputWidth;
	unsigned outputHeight;
	if(visualOutput.size() > 0) {
		outputWidth = region.grid_size.x * blockEdge;
		outputHeight = region.grid_size.y * blockEdge;
		output.reserve(outputWidth * outputHeight);
	}

	// Calculate histograms
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

	// Normalize
	int size = getHistogramLength() * getNumberHistograms();
	for(int i = 0; i < size; i++)
	{
		histograms[i] /= (blockEdge * blockEdge);
	}

	if(visualOutput.size() > 0) {
		// Save check image
		lodepng::encode(visualOutput + ".png", output, outputWidth, outputHeight, LCT_GREY);
		std::cout << output.size() << " output pixels" << std::endl;

		// Save histograms to text file
		std::ofstream file(visualOutput + ".lbp");
		file << std::fixed << std::setprecision(3);
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
