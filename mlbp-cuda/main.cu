#include <iostream>
#include <bitset>
#include <chrono>
#include <stdexcept>
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

	// Save check image
	std::vector<byte> outputImage;
	unsigned outputWidth, outputHeight;
	output.calculateNormalizedLBPs(5, 8, 64, &outputImage, &outputWidth, &outputHeight);
	lodepng::encode("test-output.png", outputImage, outputWidth, outputHeight, LCT_GREY);

	// Save text histograms
	auto histograms = output.calculateNormalizedLBPs(2, 4, 256);
	LbpImageCpu::saveHistogramsToFile(histograms, output.getHistogramLength(), output.getNumberHistograms(), "test-output.lbp");
	std::cerr << "Visual and textual output saved" << std::endl;
}


static void testAndBenchmark()
{
	LbpImageCpu image(pixels, width, height);
	LbpImageCuda d_Image(pixels, width, height);

	int samp[] = {4, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
	float rads[] = {1.0, 1.75, 1.0, 1.0, 2.0, 2.75, 3.0, 3.0, 4.0, 4.0, 4.0, 5.0, 5.0, 5.0, 6.0, 6.0};
	int edge[] = {16, 16, 16, 16, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32};
	for(int i = 0; i < 16; i++)
	{
		try {
			Benchmark::start();
			auto cpuHistograms = image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
			Benchmark::stop();
			long cpuMillis = Benchmark::getMillis();

			Benchmark::start();
			auto gpuHistograms = d_Image.calculateNormalizedLBPs(rads[i], samp[i], edge[i]);
			Benchmark::stop();
			long gpuMillis = Benchmark::getMillis();

			std::cerr << "With conf {s=" << samp[i] << "; r=" << rads[i] << "; e=" << edge[i] << "} ";
			std::cerr << "\tCPU took " << cpuMillis << "ms and GPU " << gpuMillis << "ms";

			// Test against output correctness
			long limit = image.getNumberHistograms() * image.getHistogramLength();
			for(long j = 0; j < limit; j++)
			{
				if(cpuHistograms[j] != gpuHistograms[j]) {
					throw std::logic_error("CPU and GPU outputs differ at " + std::to_string(j) + " " + std::to_string(cpuHistograms[j]) + " " + std::to_string(gpuHistograms[j]));
				}
			}
			std::cerr << "\tTest finished OK" << std::endl;
		}
		catch(const std::invalid_argument& e) {
			std::cerr << e.what() << std::endl;
			std::cerr << "Conf {s=" << samp[i] << "; r=" << rads[i] << "; e=" << edge[i] << "} is not supported" << std::endl;
			std::cerr << "End of main" << std::endl;
			return;
		}
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
	cudaDeviceReset();
	return 0;
}
