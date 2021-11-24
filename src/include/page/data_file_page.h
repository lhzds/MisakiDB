#pragma once
#include "globals.h"

namespace MisakiDB {
class DataFilePage {
private:
  static constexpr RecordSizeType INVALID_OFFSET { -1 };
  
  /* the struct of slot array element. */
  struct Slot {
    RecordSizeType m_offset;
    RecordSizeType m_length;
    bool m_isBlob;
  }__attribute__((packed));
  
  /* Number of records on the page. */
  RecordSizeType m_recordNum;
  /* Number of invalid slots in the slot array. */
  RecordSizeType m_invalidSlotNum;
  /* The end offset of the continuous free space on the page. */
  RecordSizeType m_freeSpaceEndOffset;
  
  /* Get the total number of slots in the slot array. */
  inline int getSlotsNum() const;
  /* Get start addr of the slot array. */
  inline Slot *getSlotArray();
  /* Get start addr of the slot array.(for const) */
  inline const Slot *getSlotArray() const;
  /* Get the start addr of records. */
  inline char *getRecords();
  /* Get the start addr of records. (for const)*/
  inline const char *getRecords() const;
  
  inline RecordSizeType getRecordOffset(int slotArrayIndex) const;
  inline RecordSizeType getRecordLength(int slotArrayIndex) const;
  inline bool isBlob(int slotArrayIndex) const;
  inline void setRecordOffset(int slotArrayIndex, RecordSizeType offset);
  inline void setRecordLength(int slotArrayIndex, RecordSizeType length);
  inline void setIsBlob(int slotArrayIndex, bool isBlob);

public:
  static constexpr size_t SLOT_SIZE { sizeof(Slot) };
  static constexpr size_t MIN_FIXED_SIZE { sizeof(m_recordNum) + sizeof(m_invalidSlotNum)
                                               + sizeof(m_freeSpaceEndOffset)};
  static constexpr RecordSizeType MAX_RECORD_SIZE { PAGE_SIZE - MIN_FIXED_SIZE - SLOT_SIZE };
  
  void init();
  std::string getRecord(int slotArrayNum) const;
  RecordSizeType removeRecord(int slotArrayIndex);
  RecordSizeType insertRecord(const std::string& record);
};
}
