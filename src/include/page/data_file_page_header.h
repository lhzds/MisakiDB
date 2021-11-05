#pragma once
#include "globals.h"

namespace MisakiDB {
class DataFileHeader {
private:
  PageIDType m_nextPageID = 1;
public:
  void init();
  
  PageIDType getNextPageID() const;
  void incNextPageID();
};
}