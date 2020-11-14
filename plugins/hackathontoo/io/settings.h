#pragma once

#include <string>

namespace hackathon
{
  struct Settings
  {
    std::string connectionString;
    bool overwrite = false;
    int32_t cacheSize = 10000;
    std::string imagePath;
  };
} // namespace hackathon
