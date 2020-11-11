#include "cubiomes/finders.h"
#include "types.hpp"

static int mapViableBiome(const Layer * l, int * out, int x, int z, int w, int h)
{
   int err = mapBiome(l, out, x, z, w, h);
   if U(err != 0)
       return err;

   int styp = * (const int*) l->data;
   int i, j;

   for (j = 0; j < h; j++)
   {
      for (i = 0; i < w; i++)
      {
         int biomeID = out[i + w*j];
         switch (styp)
         {
         case Swamp_Hut:
            if (biomeID == swamp)
               return 0;
            break;
         default:
            return 0;
         }
      }
   }

   return 1; // required biomes not found: set err status to stop generator
}

static int mapViableShore(const Layer * l, int * out, int x, int z, int w, int h)
{
   int err = mapShore(l, out, x, z, w, h);
   if U(err != 0)
       return err;

   int styp = * (const int*) l->data;
   int i, j;

   for (j = 0; j < h; j++)
   {
      for (i = 0; i < w; i++)
      {
         int biomeID = out[i + w*j];
         if(styp == Swamp_Hut)
            if (biomeID == swamp)
               return 0;
         else
            return 0;
      }
   }

   return 1;
}

int isViableSwampHutPos(int mcversion, LayerStack *g,
        int64_t seed, int blockX, int blockZ, int* map)
{
   int structureType = Swamp_Hut;
   Layer *l;
   int viable;
   int64_t chunkX = blockX >> 4;
   int64_t chunkZ = blockZ >> 4;
   Layer lbiome = g->layers[L_BIOME_256];
   Layer lshore = g->layers[L_SHORE_16];
   g->layers[L_BIOME_256].data = (void*) &structureType;
   g->layers[L_BIOME_256].getMap = mapViableBiome;
   g->layers[L_SHORE_16].data = (void*) &structureType;
   g->layers[L_SHORE_16].getMap = mapViableShore;
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
   //map = allocCache(l, 1, 1);
   memset(map, 0, sizeof(int)*23*23);
   if (l->getMap(l, map, blockX, blockZ, 1, 1))
      goto L_NOT_VIABLE;
   if (map[0] != swamp)
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
   return viable;
}

huts getHutPositions(int64_t seed, MCversion version, Pos begin, Pos end)
{
   huts ret;
   LayerStack g;
   setupGenerator(&g, version);
   ret.reserve((abs(begin.z - end.z)*abs(begin.x - end.x))/10);
   int* map = (int*) malloc(sizeof(int)*23*23);
   for (int x = begin.x; x < end.x; x++)
      for (int z = begin.z; z < end.z; z++)
      {
         Pos temp = getFeaturePos(SWAMP_HUT_CONFIG, seed, x, z);
         if(isViableSwampHutPos(version, &g, seed, temp.x, temp.z,map))
            ret.push_back({{x,z},0});
      }
   if (map)
      free(map);
   return ret;
}