#include "page/index_file_page_header.h"

namespace MisakiDB {

PageIDType IndexFileHeader::getNextPageId() const {
  return m_nextPageID;
}
void IndexFileHeader::setNextPageId(PageIDType nextPageId) {
  m_nextPageID = MNextPageId;
}
PageIDType IndexFileHeader::getRootPageId() const {
  return m_rootPageID;
}
void IndexFileHeader::setRootPageId(PageIDType rootPageId) {
  m_rootPageID = rootPageId;
}
PageIDType IndexFileHeader::getFreePageListHeader() const {
  return m_freePageListHeader;
}
void IndexFileHeader::setFreePageListHeader(PageIDType freePageListHeader) {
  m_freePageListHeader = freePageListHeader;
}
}
