#include "page/free_space_map_file_header.h"

namespace MisakiDB {
uint64_t FreeSpaceMapFileHeader::getFreeSpaceMapFilePageNum() const {
  return m_freeSpaceMapFilePageNum;
}
void FreeSpaceMapFileHeader::setFreeSpaceMapFilePageNum(uint64_t freeSpaceMapFilePageNum) {
  m_freeSpaceMapFilePageNum = freeSpaceMapFilePageNum;
}
}

