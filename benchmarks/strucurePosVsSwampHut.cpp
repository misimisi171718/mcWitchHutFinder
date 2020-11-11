#include "benchmark/cppbenchmark.h"

#include "cubiomes/finders.h"

int isViableSwampHutPos(int mcversion, LayerStack *g,
        int64_t seed, int blockX, int blockZ, int* map);

// Benchmark sin() call for 5 seconds (by default).
// Make 5 attemtps (by default) and choose one with the best time result.
class ContainerFixture
{
protected:
    LayerStack g;
    int* map;

    ContainerFixture()
    {
        initBiomes();
        setupGenerator(&g,MC_1_16);
        map = (int*) malloc(sizeof(int)*23*23);
    }
    ~ContainerFixture()
    {
        if(map)
            free(map);
    }
};

BENCHMARK_FIXTURE(ContainerFixture,"Structure",CppBenchmark::Settings().Attempts(3))
{
   isViableStructurePos(Swamp_Hut,MC_1_16,&g,1,1,1);
}

BENCHMARK_FIXTURE(ContainerFixture,"Swamp Hut",CppBenchmark::Settings().Attempts(3))
{
   isViableSwampHutPos(MC_1_16,&g,1,1,1,map);
}

BENCHMARK_MAIN()