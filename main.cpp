#include "lib/cubiomes/finders.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <chrono>
#include <array>

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
   for (size_t i = 0; i < input.size(); i++)
   {
      input[i].neigbours = 0;
      for (size_t j = 0; j < input.size(); j++)
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

float distance(const Pos first, const Pos second)
{
   float ret = sqrt(pow(first.x - second.x, 2) + pow(first.z - second.z, 2));
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
         positions[j] = getStructurePos(SWAMP_HUT_CONFIG,seed,i[j].position.x,i[j].position.x,NULL);
      for (size_t j = 0; j < 4; j++)
         for (size_t k = j+1; k < 4; k++)
            if (distance(positions[j],positions[k]) > 256.0)
               goto TOOFAR;

      if (false)
      {
         TOOFAR:
         bad = true;
      }
      if(!bad)
         ret.push_back(i);
   }
   return ret;
}

int main(int argc, char const *argv[])
{
   initBiomes();
   LayerStack g;
   setupGenerator(&g, MC_1_16);
   //int64_t seed = 9033216931289589291;
   int64_t seed = 17451728208755585;
   int Distance = 100;

   huts h;
   h.reserve(pow(Distance*2,2));
   for (int x = Distance * -1; x < Distance; x++)
      for (int z = Distance * -1; z < Distance; z++)
      {
         Pos temp = getStructurePos(SWAMP_HUT_CONFIG, seed, x, z, NULL);
         if(isViableStructurePos(Swamp_Hut, MC_1_16, &g, seed, temp.x, temp.z))
            h.push_back({{x,z},0});
      }

   h = filterNeigbours(h);
   h = filterNeigbours(h);
   auto j = getQuads(h);
   j = filterQuads(j,seed);
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
   std::cout << distance({446105,121721},{446745,121705}) << std::endl;
   return 0;
}
