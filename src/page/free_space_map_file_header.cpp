#include "page/free_space_map_file_header.h"

namespace MisakiDB {
uint64_t FreeSpaceMapFileHeader::getNextPageID() const {
  return m_nextPageID;
}
void FreeSpaceMapFileHeader::setNextPageID(uint64_t nextPageID) {
  m_nextPageID = nextPageID;
}
}

