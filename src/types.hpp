#pragma once
#include <vector>
#include <array>
#include "cubiomes/finders.h"

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