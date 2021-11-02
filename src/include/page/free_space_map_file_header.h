#pragma once

#include "globals.h"

namespace MisakiDB {
class FreeSpaceMapFileHeader {
private:
  uint64_t m_nextPageID = 0;
public:
  uint64_t getNextPageID() const;
  void setNextPageID(uint64_t nextPageID);
};
}


