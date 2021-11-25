#include "page/blob_file_page.h"

namespace MisakiDB {
void BlobFilePage::init() {
  m_blobListNextPageID = INVALID_PAGE_ID;
  m_dataSize = 0;
}

PageIDType BlobFilePage::getBlobListNextPageID() const {
  return m_blobListNextPageID;
}

void BlobFilePage::setBlobListNextPageID(PageIDType blobListNextPageID) {
  m_blobListNextPageID = blobListNextPageID;
}

int BlobFilePage::getDataSize() const {
  return m_dataSize;
}

void BlobFilePage::setDataSize(int dataSize) {
  m_dataSize = dataSize;
}

void BlobFilePage::writeData(const char *data, size_t dataSize) {
  m_dataSize = dataSize;
  memcpy(m_data, data, dataSize);
}

std::string BlobFilePage::readData() const {
  return std::string(m_data, m_dataSize);
}

}