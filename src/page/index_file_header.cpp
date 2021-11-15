#include "page/index_file_header.h"

namespace MisakiDB {
void IndexFileHeader::init() {
  m_nextPageID = 1;
  m_rootPageID = INVALID_PAGE_ID;
  m_freePageListHeader = INVALID_PAGE_ID;
}

PageIDType IndexFileHeader::getNextPageID() const {
  return m_nextPageID;
}

void IndexFileHeader::incNextPageID() {
  m_nextPageID += 1;
}

PageIDType IndexFileHeader::getRootPageID() const {
  return m_rootPageID;
}

void IndexFileHeader::setRootPageID(PageIDType rootPageID) {
  m_rootPageID = rootPageID;
}

PageIDType IndexFileHeader::getFreePageListHeader() const {
  return m_freePageListHeader;
}

void IndexFileHeader::setFreePageListHeader(PageIDType freePageListHeader) {
  m_freePageListHeader = freePageListHeader;
}
}
