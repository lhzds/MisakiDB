#pragma once
#include "globals.h"

namespace MisakiDB {
class DataFilePage {
private:
  class Slot {
    RecordSizeType m_offset;
    RecordSizeType m_length;
    bool m_isBlob;
  };
//  Slot *getSlot(int slotArrayIndex);
  
  RecordSizeType m_recordNum;
  RecordSizeType m_invalidSlotNum;
  RecordSizeType m_lastFreeSpaceOffset;
  
public:
  static constexpr size_t SLOT_SIZE { sizeof(Slot) };
  static constexpr size_t MIN_FIXED_SIZE { sizeof(m_recordNum) + sizeof(m_invalidSlotNum) + sizeof(m_lastFreeSpaceOffset)};
  static constexpr RecordSizeType MAX_RECORD_SIZE { PAGE_SIZE - MIN_FIXED_SIZE - SLOT_SIZE};
  
  void init();
  
//  RecordSizeType getOffset(int slotArrayIndex) const;
//  RecordSizeType getLength(int slotArrayIndex) const;
//  bool isBlob(int slotArrayIndex) const;
//  void setOffset(int slotArrayIndex, RecordSizeType offset);
//  void setLength(int slotArrayIndex, RecordSizeType length);
//  void setIsBlob(int slotArrayIndex, bool isBlob);
};
}
