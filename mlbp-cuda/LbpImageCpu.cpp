#include "LbpImageCpu.h"
#include <iostream>
#include <stdexcept>
#include "lodepng.h"

LbpImageCpu::LbpImageCpu(const std::vector<byte>& rgbPixels, unsigned width, unsigned height) : AbstractLbpImage(rgbPixels, width, height) {
	toGrayscale();
}

LbpImageCpu::~LbpImageCpu() {
}

void LbpImageCpu::toGrayscale()
{
	pixels.reserve(rgbPixels.size() / 4);
	for(int i = 0; i < rgbPixels.size(); i += 4)
	{
		byte gray = (byte)round((rgbPixels[i] + rgbPixels[i+1] + rgbPixels[i+2]) / 3.f);
		pixels.push_back(gray);
	}
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

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge)
{
	return calculateNormalizedLBPs(radius, blockEdge, "");
}

float *LbpImageCpu::calculateNormalizedLBPs(float radius, unsigned blockEdge, std::string visualOutput)
{
	// Will hold all the image's histograms
	unsigned long size = pow(2, numSamples) * region.grid_size.x * region.grid_size.y;
	float *histograms;
	try {
		histograms = new float[size];
		std::cout << "new float[" << size << "]" << std::endl;
	}
	catch(const std::bad_alloc& e) {
		std::cerr << "new float[" << size << "] allocation failed" << std::endl;
		return nullptr;
	}

	// Visual output if requested
	std::vector<byte> output;
	unsigned outputWidth;
	unsigned outputHeight;
	if(visualOutput.size() > 0) {
		outputWidth = region.grid_size.x * blockEdge;
		outputHeight = region.grid_size.y * blockEdge;
		output.reserve(4 * outputWidth * outputHeight);
	}

	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			unsigned result = compareWithNeighborhood(i, j);
			if(visualOutput.size() > 0) {
				byte check = (byte)roundf(result / powf(2.f, numSamples - 8));
				output.push_back(check);
			}
		}
	}

	if(visualOutput.size() > 0) {
		lodepng::encode(visualOutput, output, outputWidth, outputHeight, LCT_GREY);
		std::cout << output.size() << " output pixels" << std::endl;
	}
	return histograms;
}
