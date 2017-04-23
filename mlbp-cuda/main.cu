#include <iostream>
#include <bitset>
#include <chrono>
#include "lodepng.h"
#include "LbpImageCpu.h"
#include "Benchmark.h"

#define BLOCK_EDGE 	16
#define RADIUS		1.0
#define SAMPLES		8
#define BLOCK_SIZE 	(BLOCK_EDGE*BLOCK_EDGE)

static std::vector<byte> pixels;
static unsigned width;
static unsigned height;

static bool loadImage(const std::string& filename)
{
	unsigned error = lodepng::decode(pixels, width, height, filename);
	if(error) {
		std::cerr << lodepng_error_text(error) << std::endl;
		return false;
	}
	std::cout << "image size is " << width << "x" << height << std::endl;
	return true;
}

int main(int argc, char **argv) {
	// Load
	if(argc < 2) {
		std::cerr << "No image file specified" << std::endl;
		return 1;
	}
	if(!loadImage(argv[1])) {
		return 1;
	}
	LbpImageCpu image(pixels, width, height);
	if(!image.checkMinimumSize(RADIUS, BLOCK_EDGE)) {
		std::cerr << "The image is too small" << std::endl;
		return 1;
	}
	// Prepare
	image.calcSamplingOffsets(RADIUS, SAMPLES);
	image.calcImageRegion(RADIUS, BLOCK_EDGE);

	// Process
	Benchmark::start();
	image.calculateNormalizedLBPs(RADIUS, BLOCK_EDGE);
	Benchmark::stop();

	std::cout << Benchmark::getMillis() << "ms elapsed" << std::endl;
	return 0;
}
