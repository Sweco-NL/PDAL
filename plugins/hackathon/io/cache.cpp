#include "cache.h"

namespace hackathon
{
  void Cache::clear()
  {
    id.clear();
    minX.clear();
    maxX.clear();
    minY.clear();
    maxY.clear();
    minZ.clear();
    maxZ.clear();
    x.clear();
    y.clear();
    z.clear();
    classification.clear();
    intensity.clear();
    gpsTime.clear();
    pointId.clear();
    pointSourceId.clear();
  }
} // namespace hackathon
