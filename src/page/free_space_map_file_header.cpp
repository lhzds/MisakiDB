#include "page/free_space_map_file_header.h"

namespace MisakiDB {
void FreeSpaceMapFileHeader::init() {
  m_nextPageID = 1;
}

PageIDType FreeSpaceMapFileHeader::getNextPageID() const {
  return m_nextPageID;
}
void FreeSpaceMapFileHeader::incNextPageID() {
  m_nextPageID += 1;
}
}

