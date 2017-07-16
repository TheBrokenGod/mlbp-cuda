#include <iostream>
#include <bitset>
#include <chrono>
#include "lodepng.h"
#include "LbpImageCpu.h"
#include "LbpImageCuda.h"
#include "Benchmark.h"

#define BLOCK_EDGE 	32
#define RADIUS		4.0
#define SAMPLES		15

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

static void testAndBenchmark();

int main(int argc, char **argv) {
	if(argc < 2) {
		std::cerr << "No image file specified" << std::endl;
		return 1;
	}
	if(!loadImage(argv[1])) {
		return 1;
	}
	testAndBenchmark();
	return 0;
}

static void testAndBenchmark() {
	int samp[] = {4, 6, 8, 10, 12, 15};
	float rads[] = {1.0, 1.75, 1.0, 2.0, 3.0, 4.0};
	int edge[] = {8, 10, 16, 16, 32, 32};

	for(int i = 0; i < 6; i++)
	{
		// TODO move outside
		LbpImageCpu image(pixels, width, height);
		LbpImageCuda d_Image(pixels, width, height);

		Benchmark::start();
		float *histograms1 = image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
		Benchmark::stop();
		long cpuMillis = Benchmark::getMillis();
		Benchmark::start();
		float *histograms2 = d_Image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
		Benchmark::stop();
		long gpuMillis = Benchmark::getMillis();
		std::cerr << "With conf {r=" << rads[i] << ";s=" << samp[i] << ";e=" << edge[i] << "} ";
		std::cerr << "CPU took " << cpuMillis << "ms and GPU " << gpuMillis << "ms";

		// Test against output correctness
		long limit = image.getNumberHistograms() * image.getHistogramLength();
		for(long j = 0; j < limit; j++)
		{
			// Throw if CPU and GPU results differ
			if(histograms1[j] != histograms2[j]) {
				std::cerr << j << " : " << histograms1[j] << " ; " << histograms2[j] << std::endl;
				throw std::logic_error("");
			}
		}
		delete histograms1, histograms2;
		std::cerr << " - Test finished OK" << std::endl;
	}
}
