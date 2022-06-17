#pragma once

#include "globals.h"

namespace MisakiDB {
struct FreeSpaceEntry{
  uint16_t invalidSlotNum;
  uint16_t freeSpace;
};

class FreeSpaceMapFilePage {
public:
  static constexpr size_t MAX_SIZE = PAGE_SIZE / sizeof(FreeSpaceEntry);

  void init();
  
  FreeSpaceEntry getFreeSpaceEntry(int index) const;
  void addFreeSpace(int index, RecordSizeType recordSize);
  void subFreeSpace(int index, bool usedInvalidSlot, RecordSizeType recordSize);
  
private:
  FreeSpaceEntry m_freeSpaceMap[MAX_SIZE] {};
};
}

