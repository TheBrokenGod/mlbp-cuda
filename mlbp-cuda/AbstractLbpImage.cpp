#include "AbstractLbpImage.h"
#include <iostream>

AbstractLbpImage::AbstractLbpImage(const std::vector<byte>& rgbaPixels, unsigned width, unsigned height) :
	pixels(toGrayscale(rgbaPixels)),
	width(width),
	height(height),
	radius(0.f),
	samples(0),
	blockEdge(0),
	histograms(nullptr) {
}

AbstractLbpImage::~AbstractLbpImage() {
}

std::vector<byte> AbstractLbpImage::toGrayscale(const std::vector<byte>& rgbaPixels)
{
	std::vector<byte> pixels;
	pixels.reserve(rgbaPixels.size() / 4);

	for(int i = 0; i < rgbaPixels.size(); i += 4)
	{
		byte gray = (byte)round((rgbaPixels[i] + rgbaPixels[i+1] + rgbaPixels[i+2]) / 3.f);
		pixels.push_back(gray);
	}

	return pixels;
}

void AbstractLbpImage::prepare(float radius, unsigned samples, unsigned blockEdge)
{
	this->radius = radius;
	this->samples = samples;
	this->blockEdge = blockEdge;

	if(!checkMinimumSize()) {
		throw new std::invalid_argument("The image is too small");
	}
	calcSamplingOffsets();
	calcImageRegion();
}

bool AbstractLbpImage::checkMinimumSize()
{
	int minSize = 2 * ((int)round(radius)) + blockEdge;

	if(width >= minSize && height >= minSize) {
		return true;
	}
	return false;
}

void AbstractLbpImage::calcSamplingOffsets()
{
	// Loop the unit circle clockwise
	float angle = 2 * M_PI / samples;
	for(int sample = 0; sample < samples; sample++)
	{
		offsets.push_back({0, 0});
		offsets[sample].y = (-1) * (int)round(radius * cos(sample * angle));
		offsets[sample].x = (+1) * (int)round(radius * sin(sample * angle));

		// TODO
		std::cout << "sample at " << offsets[sample].y << " ; " << offsets[sample].x << std::endl;
	}
	samples = offsets.size();
}

void AbstractLbpImage::calcImageRegion()
{
	int radiusInt = (int)round(radius);

	// Define the processed sub-region of the image
	region.gaps_pixel.y = (2 * radiusInt + (height - 2 * radiusInt) % blockEdge) / 2;
	region.gaps_pixel.x = (2 * radiusInt + (width - 2 * radiusInt) % blockEdge) / 2;
	region.grid_size.y  = (height - 2 * radiusInt) / blockEdge;
	region.grid_size.x  = (width - 2 * radiusInt) / blockEdge;
	region.end_pixels.y = region.gaps_pixel.y + region.grid_size.y * blockEdge;
	region.end_pixels.x = region.gaps_pixel.x + region.grid_size.x * blockEdge;

	// TODO
	std::cout << "grid is " << region.grid_size.x << "x" << region.grid_size.y;
	std::cout << " with gaps " << region.gaps_pixel.x << "x" << region.gaps_pixel.y;
	std::cout << " and " << (width - region.end_pixels.x);
	std::cout << "x" << (height - region.end_pixels.y) << std::endl;
}
