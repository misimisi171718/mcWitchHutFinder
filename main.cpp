#include "lib/cubiomes/finders.h"
#include <iostream>
#include <vector>


int main(int argc, char const *argv[])
{
   initBiomes();
   LayerStack g;
   setupGenerator(&g, MC_1_16);
   int64_t seed = 9033216931289589291;
   int distance = 100;
   std::vector<Pos> huts;
   for (int x = distance * -1; x < distance; x++)
      for (int z = distance * -1; z < distance; z++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, 3, 0, NULL);
         if(isViableStructurePos(Swamp_Hut, MC_1_16, &g, seed, x, z))
            huts.push_back({x,z});
      }
   for (auto &&i : huts)
      std::cout << i.x << "; " << i.z << std::endl;
   
   return 0;
}
