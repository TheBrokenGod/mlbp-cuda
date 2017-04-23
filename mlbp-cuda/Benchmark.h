#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <chrono>

class Benchmark
{
private:
	static std::chrono::time_point<std::chrono::high_resolution_clock> _start;
	static std::chrono::time_point<std::chrono::high_resolution_clock> _stop;

public:
	static void start();
	static void stop();
	static long getSeconds();
	static long getMillis();
};

#endif
