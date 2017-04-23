#include "LbpImageCpu.h"
#include <iostream>

LbpImageCpu::LbpImageCpu(const std::vector<byte>& pixels, unsigned width, unsigned height) : AbstractLbpImage(pixels, width, height) {
}

LbpImageCpu::~LbpImageCpu() {
}

void LbpImageCpu::toGrayscale()
{
	grayPixels.reserve(pixels.size() / 4);
	for(int i = 0; i < pixels.size(); i += 4)
	{
		byte gray = (byte)round((pixels[i] + pixels[i+1] + pixels[i+2]) / 3.f);
		grayPixels.push_back(gray);
	}
}

byte LbpImageCpu::pixelAt(unsigned row, unsigned col)
{
	return grayPixels[row * width + col];
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
	toGrayscale();

	std::vector<unsigned> junk;
	for(int i = region.gaps_pixel.y; i < region.end_pixels.y; i++)
	{
		for(int j = region.gaps_pixel.x; j < region.end_pixels.x; j++)
		{
			unsigned result = compareWithNeighborhood(i, j);
			junk.push_back(result);
//			std::cout << result << std::endl;

//			TODO
//			pixels[4 * (i * width + j) + 1] = sample(i, j);
		}
	}
//	lodepng::encode("image_out.png", pixels, width, height);
	std::cout << junk.size() << std::endl;
	return nullptr;
}
