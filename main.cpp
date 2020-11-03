#include "lib/cubiomes/finders.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>

struct huts
{
   std::vector<Pos> huts;
   std::vector<uint8_t> neigbours;
};

bool isNeigbour(const Pos &firs, const Pos &second)
{
   int zdist = abs(firs.z-second.z);
   int xDist = abs(firs.x-second.x);
   if(xDist+zdist == 1) return true;
   if (xDist == 1 && zdist == 1) return true;
   return false;
}

huts filterNeigbours(huts& input)
{
   input.neigbours.reserve(input.huts.size());
   for (size_t i = 0; i < input.huts.size(); i++)
   {
      input.neigbours[i] = 0;
      for (size_t j = 0; j < input.huts.size(); j++)
      {
         if(isNeigbour(input.huts[i],input.huts[j]))
            input.neigbours[i]++;
      }
   }

   huts ret;
   for (size_t i = 0; i < input.huts.size() ; i++)
      if(input.neigbours[i] >= 3)
      {
         ret.huts.push_back(input.huts[i]);
         ret.neigbours.push_back(input.neigbours[i]);
      }
   return ret;
}

int main(int argc, char const *argv[])
{
   initBiomes();
   LayerStack g;
   setupGenerator(&g, MC_1_16);
   int64_t seed = 9033216931289589291;
   int distance = 500;

   auto start = std::chrono::steady_clock::now();

   std::vector<Pos> Huts;
   for (int x = distance * -1; x < distance; x++)
      for (int z = distance * -1; z < distance; z++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, x, z, NULL);
         if(isViableStructurePos(Swamp_Hut, MC_1_16, &g, seed, temp.x, temp.z))
            Huts.push_back({x,z});
      }

   auto end = std::chrono::steady_clock::now();
   std::cout << "Elapsed time in milliseconds : "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
		<< " ms" << std::endl;

   huts old;
   old.huts = std::move(Huts);
   auto temp = filterNeigbours(old);
   temp = filterNeigbours(temp);

   for (auto &&i : temp.huts)
   {
      std::cout << i.x << "; " << i.z << std::endl;
   }
   
   return 0;
}
