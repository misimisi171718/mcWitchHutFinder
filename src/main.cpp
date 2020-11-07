#include "cubiomes/finders.h"
#include "types.hpp"
#include "getHutPosition.hpp"
#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>
#include <array>
#include <tuple>
#include <string>
#include <thread>
#include <future>

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
   for (size_t i = 0; i < input.size(); i++)
   {
      quadHut temp;
      temp[0] = input[i];
      size_t found = 1;

      for (size_t j = i; input[i].position.x + 1 >= input[j].position.x && j < input.size(); j++)
      {
         if(temp[0].position.x+1 == input[j].position.x && temp[0].position.z == input[j].position.z)
         {
            temp[1] = input[j];
            found++;
         }
         if(temp[0].position.x == input[j].position.x && temp[0].position.z + 1 == input[j].position.z)
         {
            temp[2] = input[j];
            found++;
         }
         if(temp[0].position.x+1 == input[j].position.x && temp[0].position.z +1 == input[j].position.z)
         {
            temp[3] = input[j];
            found++;
         }
      }
      
      if(found == 4)
      {
         ret.push_back(temp);
      }
      
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
   initBiomes();

   auto start = std::chrono::high_resolution_clock::now();

   auto threadCount = std::thread::hardware_concurrency();
   huts h;
   if (threadCount == 0)
   {
      h = getHutPositions(seed,Distance,version,-Distance,Distance);
      std::cerr << "thread count coud not be determend using a single thread" << std::endl;
   }
   else
   {
      std::vector<std::future<huts>> threads;
      std::vector<huts> threadRetuns;
      threads.reserve(threadCount);
      for (size_t i = 0; i < threadCount; i++)
      {
         int start = floor(-(float)Distance+( i   *(2*(float)Distance/(float)threadCount)));
         int end   = floor(-(float)Distance+((i+1)*(2*(float)Distance/(float)threadCount)));
         threads.push_back(std::async(getHutPositions,seed,Distance,version,start,end));
      }
      for (size_t i = 0; i < threadCount; i++)
         threadRetuns.push_back(threads[i].get());
      int numHuts = 0;
      for (auto &&i : threadRetuns)
         numHuts += i.size();
      h.reserve(numHuts);
      for (auto &&i : threadRetuns)
         h.insert(h.end(),i.begin(),i.end());
   }
   

   auto end = std::chrono::high_resolution_clock::now();
   std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() 
      << "ms" << std::endl;

   start = std::chrono::high_resolution_clock::now();

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
