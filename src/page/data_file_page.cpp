#include "page/data_file_page.h"

namespace MisakiDB {
void DataFilePage::init() {
  m_recordNum = 0;
  m_invalidSlotNum = 0;
  m_lastFreeSpaceOffset = PAGE_SIZE - 1;
}
}