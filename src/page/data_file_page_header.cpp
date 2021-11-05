#include "page/data_file_page_header.h"

namespace MisakiDB {
void DataFileHeader::init() {
  m_nextPageID = 1;
}

PageIDType DataFileHeader::getNextPageID() const {
  return m_nextPageID;
}
void DataFileHeader::incNextPageID() {
  m_nextPageID += 1;
}
}