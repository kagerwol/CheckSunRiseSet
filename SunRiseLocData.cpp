#include <vector>
#include <string>
#include "SunRiseLocData.h"

const SunRiseLocData *isKnownLocation(const std::string& shortName, const std::vector<SunRiseLocData>& locations)
{
  for (const auto& loc : locations)
  {
    if (loc.getShortName() == shortName)
    {
      return &loc;
    }
  }
  return nullptr;
}

const SunRiseLocData* getDefaultLocation(const std::vector<SunRiseLocData>& locations)
{
  for (const auto& loc : locations)
  {
    if (loc.isDefault())
    {
      return &loc;
    }
  }
  return nullptr;
}