#pragma once

#include "globals.h"

namespace MisakiDB {
class FreeSpaceMapFilePage {
public:
  static constexpr size_t MAX_SIZE = PAGE_SIZE / sizeof(RecordSizeType);
  
  void init();
  
  RecordSizeType getFreeSpace(int index) const;
  void addFreeSpace(int index, RecordSizeType spaceSize);
  void subFreeSpace(int index, RecordSizeType spaceSize);
  
private:
  RecordSizeType m_freeSpaceMap[MAX_SIZE] {};
};
}

