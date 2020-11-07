#include "cubiomes/finders.h"
#include "types.hpp"

huts getHutPositions(int64_t seed,  int distance, MCversion version, int begin, int end)
{
   huts ret;
   LayerStack g;
   setupGenerator(&g, version);
   ret.reserve(pow(distance*2,2)/10);
   for (int x = begin; x < end; x++)
      for (int z = distance * -1; z < distance; z++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, x, z, NULL);
         if(isViableStructurePos(Swamp_Hut, version, &g, seed, temp.x, temp.z))
            ret.push_back({{x,z},0});
      }
   return ret;
}