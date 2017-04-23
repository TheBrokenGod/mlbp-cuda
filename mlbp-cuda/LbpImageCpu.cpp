#include "LbpImageCpu.h"
#include <iostream>
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
	for(int i = 0; i < offsets.size(); i++)
	{
		unsigned nrow = row + offsets[i].y;
		unsigned ncol = col + offsets[i].x;
		byte neighbor = pixelAt(nrow, ncol);

		// Write one if lower
		if(pixel < neighbor) {
			int shift = offsets.size() - 1 - i;
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
				byte check = (byte)roundf(result * powf(2.f, 8 - offsets.size()));
				output.push_back(check);
			}
		}
	}

	if(visualOutput.size() > 0) {
		lodepng::encode(visualOutput, output, outputWidth, outputHeight, LCT_GREY);
	}
	std::cout << output.size() << std::endl;
	return nullptr;
}
