#include <iostream>
#include <bitset>
#include <chrono>
#include "lodepng.h"
#include "LbpImageCpu.h"
#include "LbpImageCuda.h"
#include "Benchmark.h"

#define BLOCK_EDGE 	64
#define RADIUS		3.0
#define SAMPLES		12

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
	pixels.push_back(0xff);
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
	LbpImageCuda d_Image(pixels, width, height);

	LbpImageCpu image(pixels, width, height);
	pixels.clear();
	image.prepare(RADIUS, SAMPLES, BLOCK_EDGE);

	// Process
	Benchmark::start();
	delete [] image.calculateNormalizedLBPs("test-output");
	Benchmark::stop();

	std::cout << Benchmark::getMillis() << "ms elapsed" << std::endl;
	return 0;
}
