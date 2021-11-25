#include "page/blob_file_header.h"

namespace MisakiDB {
void BlobFileHeader::init() {
  m_nextPageID = 1;
  m_freePageListHeader = INVALID_PAGE_ID;
}

PageIDType BlobFileHeader::getNextPageID() const {
  return m_nextPageID;
}

void BlobFileHeader::incNextPageID() {
  m_nextPageID += 1;
}

PageIDType BlobFileHeader::getFreePageListHeader() const {
  return m_freePageListHeader;
}

void BlobFileHeader::setFreePageListHeader(PageIDType freePageListHeader) {
  m_freePageListHeader = freePageListHeader;
}
}
