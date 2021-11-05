#pragma once
#include "globals.h"

namespace MisakiDB {
class Options final {
public:
  Options();
  size_t getIndexBufferPoolSize() const;
  size_t getDataBufferPoolSize() const;
};
}

