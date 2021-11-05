#pragma once

#include "globals.h"

namespace MisakiDB {
class IndexFileHeader {
private:
  PageIDType m_nextPageID = 1;
  PageIDType m_rootPageID = INVALID_PAGE_ID;
  PageIDType m_freePageListHeader = INVALID_PAGE_ID;
  
public:
  void init();
  
  PageIDType getRootPageID() const;
  void setRootPageID(PageIDType rootPageId);
  PageIDType getFreePageListHeader() const;
  void setFreePageListHeader(PageIDType freePageListHeader);
  PageIDType getNextPageID() const;
  void incNextPageID();
};
}
