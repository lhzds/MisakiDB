#pragma once

#include "globals.h"

namespace MisakiDB {
class FreeSpaceMapFileHeader {
private:
  PageIDType m_nextPageID = 1;
public:
  PageIDType getNextPageID() const;
  void setNextPageID(PageIDType nextPageID);
};
}


