#include "lib/cubiomes/finders.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>
#include <array>
#include <tuple>
#include <string>

struct hut
{
   Pos position;
   uint8_t neigbours = 0;

   bool operator==(const hut& other) const {
      return position.x == other.position.x && position.z == other.position.z &&
         neigbours == other.neigbours;
   }
};

typedef std::vector<hut> huts;
typedef std::array<hut,4> quadHut;
typedef std::vector<quadHut> quadHuts;

std::tuple<int64_t,int32_t,MCversion> parseArguments(const int argc, const char *argv[])
{
   if(argc < 3 && argc > 3)
   {
      std::cout << "usage: quadWichHutFinder [SEED] [SEARCH RADIUS] [MINECRAFT VERSION]" << std::endl;
      std::exit(0);
   }


   int64_t seed;
   try
   {
      seed = std::stol(argv[1]);
   }
   catch(const std::exception& e)
   {
      std::cerr << "seed must only contain numbers" << std::endl;
      std::exit(1);
   }
   
   int32_t radius;
   try
   {
      radius = std::stoi(argv[2]);
   }
   catch(const std::exception& e)
   {
      std::cerr << "radius must only contain numbers" << std::endl;
      std::exit(1);
   }
   
   MCversion version;

   if(argc == 4)
      try
      {
         version = static_cast<MCversion>(std::stoi(argv[3]) - 7);
      }
      catch(const std::exception& e)
      {
         std::cerr << "minecraft version can be specified by just just adding the main version number" << std::endl
            << "example: 1.16.4 -> 16" << std::endl;
         std::exit(1);
      }
      
   else
   {
      std::cout << "minecarft version not specified defaulting to 1.16" << std::endl;
      version = MC_1_16;
   }
   return std::make_tuple(seed,radius,version);
}

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
   int prevXPos1 = 0;
   int prevXPos2 = 0;
   int XVal = input[0].position.x;
   for (size_t i = 0; i < input.size(); i++)
   {
      if (input[i].position.x != XVal)
      {
         XVal = input[i].position.x;
         prevXPos2 = prevXPos1;
         prevXPos1 = i;
      }
      
      input[i].neigbours = 0;
      for (size_t j = prevXPos2; input[i].position.x + 1 >= input[j].position.x && j < input.size(); j++)
         if(isNeigbour(input[i].position, input[j].position))
            input[i].neigbours++;
   }

   huts ret;
   ret.reserve(input.size()/10);
   for (size_t i = 0; i < input.size() ; i++)
      if(input[i].neigbours >= 3)
         ret.push_back(input[i]);
   return ret;
}

quadHuts getQuads(huts& input)
{
   quadHuts ret;
   while (input.size() != 0)
   {
      quadHut temp;
      temp[0] = input[0];
      size_t i = 0;
      size_t found = 1;

      while (input[i].position.z - input[0].position.z < 2 && i < input.size())
      {
         if(temp[0].position.x+1 == input[i].position.x && temp[0].position.z == input[i].position.z)
         {
            temp[1] = input[i];
            found++;
         }
         if(temp[0].position.x == input[i].position.x && temp[0].position.z + 1 == input[i].position.z)
         {
            temp[2] = input[i];
            found++;
         }
         if(temp[0].position.x+1 == input[i].position.x && temp[0].position.z +1 == input[i].position.z)
         {
            temp[3] = input[i];
            found++;
         }
         i++;
      }
      
      if(found == 4)
      {
         ret.push_back(temp);
         for (size_t i = 0; i < 4; i++)
         {
            int j = 0;
            while (true)
            {
               if(input[j] == temp[i])
               {
                  input.erase(input.begin() + j);
                  break;
               }
               j++;
            }
         }
      }
      else
         input.erase(input.begin());
      
   }
   return ret;
}

int distance(const Pos first, const Pos second)
{
   int ret = pow(first.x - second.x, 2) + pow(first.z - second.z, 2);
   return ret;
}

quadHuts filterQuads(quadHuts& input,const int64_t seed)
{
   quadHuts ret;
   for (auto &&i : input)
   {
      bool bad = false;
      std::array<Pos,4> positions;
      for (size_t j = 0; j < 4; j++)
         positions[j] = getStructurePos(SWAMP_HUT_CONFIG,seed,i[j].position.x,i[j].position.z,NULL);
      for (size_t j = 0; j < 4; j++)
         for (size_t k = j+1; k < 4; k++)
            if (distance(positions[j],positions[k]) >= 256 * 256)
               bad = true;

      if(!bad)
         ret.push_back(i);
   }
   return ret;
}

int main(int argc, char const *argv[])
{
   auto [seed,Distance,version] = parseArguments(argc, argv);
   //int64_t seed = 17451728208755585;
   //int Distance = 200;
   //MCversion version = MC_1_16;
   initBiomes();
   LayerStack g;
   setupGenerator(&g, version);
   //int64_t seed = 9033216931289589291;
   applySeed(&g,seed);

   auto start = std::chrono::high_resolution_clock::now();

   huts h;
   h.reserve(pow(Distance*2,2));
   for (int x = Distance * -1; x < Distance; x++)
      for (int z = Distance * -1; z < Distance; z++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, x, z, NULL);
         if(isViableStructurePos(Swamp_Hut, version, &g, seed, temp.x, temp.z))
            h.push_back({{x,z},0});
      }

   auto end = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
      << "ms" << std::endl;

   start = std::chrono::high_resolution_clock::now();

   h = filterNeigbours(h);
   h = filterNeigbours(h);
   auto j = getQuads(h);
   j = filterQuads(j,seed);

   end = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
      << "ms" << std::endl;

   for (size_t i = 0; i < j.size(); i++)
   {
      std::cout << "[" << std::endl;
      for (size_t k = 0; k < 4; k++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, j[i][k].position.x, j[i][k].position.z, NULL);
         std::cout << "\t[" << temp.x << "; " << temp.z << "]" << std::endl;
      }
      std::cout << "]" << std::endl;
   }
   return 0;
}
