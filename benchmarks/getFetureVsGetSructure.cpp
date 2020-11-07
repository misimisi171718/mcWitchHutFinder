#include "benchmark/cppbenchmark.h"

#include "cubiomes/finders.h"

// Benchmark sin() call for 5 seconds (by default).
// Make 5 attemtps (by default) and choose one with the best time result.
BENCHMARK("structure")
{
    getStructurePos(SWAMP_HUT_CONFIG,1,1,1,NULL);
}

BENCHMARK("feature")
{
    getFeaturePos(SWAMP_HUT_CONFIG,1,1,1);
}

BENCHMARK_MAIN()