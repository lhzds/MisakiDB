#include "page/blob_file_page.h"

namespace MisakiDB {
void BlobFilePage::init() {
  m_isBlobListHead = false;
  m_blobListNextPageID = INVALID_PAGE_ID;
  m_dataSize = 0;
  m_blobID = INVALID_BLOB_ID;
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

bool BlobFilePage::isBlobListHead() const {
  return m_isBlobListHead;
}

void BlobFilePage::setIsBlobListHead(bool isBlobListHead) {
  m_isBlobListHead = isBlobListHead;
}

BlobIDType BlobFilePage::getBlobID() const {
  return m_blobID;
}

void BlobFilePage::setBlobID(BlobIDType blobID) {
  m_blobID = blobID;
}

void BlobHeadPage::writeData(const char *data, size_t dataSize) {
  setDataSize(dataSize);
  memcpy(m_data, data, dataSize);
}

std::string BlobHeadPage::readData() const {
  return std::string(m_data, getDataSize());
}

std::string BlobHeadPage::getKey() const {
  return m_key.toStr();
}

void BlobHeadPage::setKey(const std::string &key) {
  m_key.setData(key);
}

void BlobNonHeadPage::writeData(const char *data, size_t dataSize) {
  setDataSize(dataSize);
  memcpy(m_data, data, dataSize);
}

std::string BlobNonHeadPage::readData() const {
  return std::string(m_data, getDataSize());
}
}