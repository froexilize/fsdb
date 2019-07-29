#include <benchmark/benchmark.h>
#include <fsdb/fsdb.hpp>

Fsdb fsdb;

static void BM_random_insertion(benchmark::State &state) {
	for(auto _:state) {
		unsigned char buffer[state.range(0)];
		auto start = std::chrono::high_resolution_clock::now();
		fsdb.insert("VO_random", (const char *) buffer, state.range(0));
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed_seconds = std::chrono::duration_cast<std::chrono::duration<double> >(end-start);
		state.SetIterationTime(elapsed_seconds.count());
		fsdb.del("VO_random");
	}
}
BENCHMARK(BM_random_insertion)
->Unit(benchmark::kMillisecond)
->RangeMultiplier(2)
->Range(1, 1<<22)
->UseManualTime();

//BENCHMARK_MAIN();
int main(int argc, char *argv[]) {
	fsdb.init("BM_test");
	::benchmark::Initialize(&argc, argv);
	::benchmark::RunSpecifiedBenchmarks();
	fsdb.deinit();
}
