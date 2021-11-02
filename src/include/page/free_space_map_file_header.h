#pragma once

#include "globals.h"

namespace MisakiDB {
class FreeSpaceMapFileHeader {
private:
  uint64_t m_freeSpaceMapFilePageNum = 0;
public:
  uint64_t getFreeSpaceMapFilePageNum() const;
  void setFreeSpaceMapFilePageNum(uint64_t freeSpaceMapFilePageNum);
};
}


