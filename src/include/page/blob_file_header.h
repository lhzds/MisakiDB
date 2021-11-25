#pragma once
#include "globals.h"

namespace MisakiDB {
class BlobFileHeader {
private:
  PageIDType m_nextPageID = 1;
  PageIDType m_freePageListHeader = INVALID_PAGE_ID;

public:
  void init();
  
  PageIDType getNextPageID() const;
  void incNextPageID();
  PageIDType getFreePageListHeader() const;
  void setFreePageListHeader(PageIDType freePageListHeader);
};
}