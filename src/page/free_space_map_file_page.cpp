#include "page/free_space_map_file_page.h"
#include "page/data_file_page.h"

namespace MisakiDB {
void FreeSpaceMapFilePage::init() {
  for (int i = 0; i < MAX_SIZE; ++i) {
    m_freeSpaceMap[i] = PAGE_SIZE - DataFilePage::MIN_FIXED_SIZE;
  }
}

RecordSizeType FreeSpaceMapFilePage::getFreeSpace(int index) const {
  assert(0 <= index && index < MAX_SIZE);
  return m_freeSpaceMap[index];
}

void FreeSpaceMapFilePage::addFreeSpace(int index, RecordSizeType spaceSize) {
  assert(0 <= index && index < MAX_SIZE);
  m_freeSpaceMap[index] += spaceSize;
}

void FreeSpaceMapFilePage::subFreeSpace(int index, RecordSizeType spaceSize) {
  assert(0 <= index && index < MAX_SIZE);
  m_freeSpaceMap[index] -= spaceSize;
}
}
