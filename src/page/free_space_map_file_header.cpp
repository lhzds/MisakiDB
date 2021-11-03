#include "page/free_space_map_file_header.h"

namespace MisakiDB {
PageIDType FreeSpaceMapFileHeader::getNextPageID() const {
  return m_nextPageID;
}
void FreeSpaceMapFileHeader::setNextPageID(PageIDType nextPageID) {
  m_nextPageID = nextPageID;
}
}

