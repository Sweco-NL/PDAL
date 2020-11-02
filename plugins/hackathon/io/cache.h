#pragma once

#include <QVariantList>

namespace hackathon
{
  struct Cache
  {
    QVariantList id;
    QVariantList minX;
    QVariantList maxX;
    QVariantList minY;
    QVariantList maxY;
    QVariantList minZ;
    QVariantList maxZ;
    QVariantList x;
    QVariantList y;
    QVariantList z;
    QVariantList classification;
    QVariantList intensity;
    QVariantList gpsTime;
    QVariantList pointId;
    QVariantList pointSourceId;

    void clear();
  };
} // namespace hackathon
