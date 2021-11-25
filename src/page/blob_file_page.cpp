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

}