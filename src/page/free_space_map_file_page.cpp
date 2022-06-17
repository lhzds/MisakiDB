#include "page/free_space_map_file_page.h"
#include "page/data_file_page.h"

namespace MisakiDB {
void FreeSpaceMapFilePage::init() {
  for (int i = 0; i < MAX_SIZE; ++i) {
	m_freeSpaceMap[i].invalidSlotNum = 0;
    m_freeSpaceMap[i].freeSpace = PAGE_SIZE - DataFilePage::MIN_FIXED_SIZE;
  }
}

FreeSpaceEntry FreeSpaceMapFilePage::getFreeSpaceEntry(int index) const {
  assert(0 <= index && index < MAX_SIZE);
  return m_freeSpaceMap[index];
}

void FreeSpaceMapFilePage::addFreeSpace(int index, RecordSizeType recordSize) {
  assert(0 <= index && index < MAX_SIZE);
  m_freeSpaceMap[index].freeSpace += recordSize;
  ++m_freeSpaceMap[index].invalidSlotNum;
}

void FreeSpaceMapFilePage::subFreeSpace(int index, bool usedInvalidSlot, RecordSizeType recordSize) {
  assert(0 <= index && index < MAX_SIZE);
  if (usedInvalidSlot) {
	m_freeSpaceMap[index].freeSpace -= recordSize;
	--m_freeSpaceMap[index].invalidSlotNum;
  } else {
	m_freeSpaceMap[index].freeSpace -= recordSize + DataFilePage::SLOT_SIZE;
  }
}
}
