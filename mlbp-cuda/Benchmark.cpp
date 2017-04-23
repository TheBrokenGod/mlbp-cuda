#include "Benchmark.h"

std::chrono::time_point<std::chrono::high_resolution_clock> Benchmark::_start;
std::chrono::time_point<std::chrono::high_resolution_clock> Benchmark::_stop;

void Benchmark::start() {
	_start = std::chrono::high_resolution_clock::now();
}

void Benchmark::stop() {
	_stop = std::chrono::high_resolution_clock::now();
}

long Benchmark::getSeconds() {
	return std::chrono::duration_cast<std::chrono::seconds>(_stop - _start).count();
}

long Benchmark::getMillis() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(_stop - _start).count();
}
