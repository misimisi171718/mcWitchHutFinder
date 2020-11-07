#include "cubiomes/finders.h"
#include "types.hpp"

int isViableSwampHutPos(int structureType, int mcversion, LayerStack *g,
        int64_t seed, int blockX, int blockZ)
{
   int *map = NULL;
   Layer *l;
   int biome;
   int viable;
   int64_t chunkX = blockX >> 4;
   int64_t chunkZ = blockZ >> 4;
   Layer lbiome = g->layers[L_BIOME_256];
   Layer lshore = g->layers[L_SHORE_16];
   g->layers[L_BIOME_256].data = (void*) &structureType;
   //g->layers[L_BIOME_256].getMap = mapViableBiome;
   g->layers[L_SHORE_16].data = (void*) &structureType;
   //g->layers[L_SHORE_16].getMap = mapViableShore;
   if (mcversion < MC_1_16)
   {
      l = &g->layers[L_VORONOI_ZOOM_1];
   }
   else
   {  // In 1.16 the position and layer for the biome dependence changed
      // to the centre of a chunk at scale 4. Using L_RIVER_MIX_4
      // (without ocean type) should be fine for ruins and wrecks.
      l = &g->layers[L_RIVER_MIX_4];
      blockX = (chunkX << 2) + 2;
      blockZ = (chunkZ << 2) + 2;
   }
   setWorldSeed(l, seed);
   map = allocCache(l, 1, 1);
   if (genArea(l, map, blockX, blockZ, 1, 1))
      goto L_NOT_VIABLE;
   if (!isViableFeatureBiome(structureType, map[0]))
      goto L_NOT_VIABLE;
   goto L_VIABLE;

L_NOT_VIABLE:
   viable = 0;
   if (0) {
L_VIABLE:
      viable = 1;
   }
   g->layers[L_BIOME_256] = lbiome;
   g->layers[L_SHORE_16] = lshore;
   if (map)
      free(map);
   return viable;
}

huts getHutPositions(int64_t seed,  int distance, MCversion version, int begin, int end)
{
   huts ret;
   LayerStack g;
   setupGenerator(&g, version);
   ret.reserve(pow(distance*2,2)/10);
   for (int x = begin; x < end; x++)
      for (int z = distance * -1; z < distance; z++)
      {
         Pos temp = getFeaturePos(SWAMP_HUT_CONFIG, seed, x, z);
         if(isViableStructurePos(Swamp_Hut, version, &g, seed, temp.x, temp.z))
            ret.push_back({{x,z},0});
      }
   return ret;
}