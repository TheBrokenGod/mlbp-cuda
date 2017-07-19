#include <iostream>
#include <bitset>
#include <chrono>
#include "lodepng.h"
#include "LbpImageCpu.h"
#include "LbpImageCuda.h"
#include "Benchmark.h"

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

static void makeSampleOutput()
{
	LbpImageCpu output(pixels, width, height);
	delete output.calculateNormalizedLBPs(1, 8, 64, "test-output");
	std::cerr << "Visual LBPs representation saved" << std::endl;
}


static void testAndBenchmark()
{
	LbpImageCpu image(pixels, width, height);
	LbpImageCuda d_Image(pixels, width, height);

	int samp[] = {4, 6, 8, 10, 12, 15};
	float rads[] = {1.0, 1.75, 1.0, 2.0, 3.0, 4.0};
	int edge[] = {8, 10, 16, 16, 32, 32};
	for(int i = 0; i < 6; i++)
	{
		Benchmark::start();
		float *cpuHistograms = image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
		Benchmark::stop();
		long cpuMillis = Benchmark::getMillis();

		Benchmark::start();
		float *gpuHistograms = d_Image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
		Benchmark::stop();
		long gpuMillis = Benchmark::getMillis();

		std::cerr << "With conf {r=" << rads[i] << ";s=" << samp[i] << ";e=" << edge[i] << "} ";
		std::cerr << "CPU took " << cpuMillis << "ms and GPU " << gpuMillis << "ms";

		// Test against output correctness
		long limit = image.getNumberHistograms() * image.getHistogramLength();
		for(long j = 0; j < limit; j++)
		{
			if(cpuHistograms[j] != gpuHistograms[j]) {
				throw std::logic_error("CPU and GPU outputs differ");
			}
		}
		delete cpuHistograms, gpuHistograms;
		std::cerr << " - Test finished OK" << std::endl;
	}
}

int main(int argc, char **argv) {
	if(argc < 2) {
		std::cerr << "No image file specified" << std::endl;
		return 1;
	}
	if(!loadImage(argv[1])) {
		return 1;
	}
	makeSampleOutput();
	testAndBenchmark();
	return 0;
}
