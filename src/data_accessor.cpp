#include "data_accessor.h"
#include "page/data_file_page.h"
#include "page/blob_file_page.h"
#include "page/blob_file_free_page.h"
#include "globals.h"

namespace MisakiDB{
DataAccessor::DataAccessor(DataFileManager *dataFileManager)
    :m_dataFileManager(dataFileManager) {}

std::optional<std::string> DataAccessor::getRecordValue(const std::string &key, RecordIDType recordID) const {
  assert(key.size() == RECORD_KEY_SIZE);
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->rLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  auto result = targetPage->getRecordValue(key, slotArrayIndex);
  if (!result.has_value()) {
    return std::nullopt;
  }
  const auto &[recordValue, isBlob] = result.value();
  rawTargetPage->rUnlatch();
  m_dataFileManager->unpinDataPage(pageID, false);
  if (isBlob) {
    return getBlob(key, recordValue);
  }
  return recordValue;
}

void DataAccessor::removeRecord(RecordIDType recordID) {
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->wLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  auto removeResult = targetPage->removeRecord(slotArrayIndex);
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(pageID, true);
  
  if (removeResult.index() == 0) { // if the record is slob
    m_dataFileManager->addFreeSpace(pageID, std::get<RecordSizeType>(removeResult));
  } else {
    std::string blobFirstPageID = std::move(std::get<std::string>(removeResult));
    m_dataFileManager->addFreeSpace(pageID, blobFirstPageID.size());
    removeBlob(blobFirstPageID);
  }
}

RecordIDType DataAccessor::insertRecord(const std::string &key, const std::string &value) {
  assert(key.size() == RECORD_KEY_SIZE);
  Page *rawTargetPage;
  int slotArrayIndex;
  std::string record;
  bool isBlob;
  if (DataFilePage::MAX_RECORD_SIZE < value.size()) { // if value is blob
    std::string blobFirstPageID = insertBlob(key, value);
    record = key + blobFirstPageID;
    isBlob = true;
  } else {
    record = key + value;
    isBlob = false;
  }
  rawTargetPage = m_dataFileManager->allocateDataPage(record.size());
  rawTargetPage->wLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  slotArrayIndex = targetPage->insertRecord(record, isBlob);
  
  RecordIDType newRecordID;
  newRecordID.pageID = rawTargetPage->getPageID();
  newRecordID.slotArrayIndex = slotArrayIndex;
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(newRecordID.pageID, true);
  return newRecordID;
}

std::string DataAccessor::insertBlob(const std::string &key, const std::string &value) {
  Page *prevPage = nullptr;
  PageIDType firstPageID;
  size_t insertedSize = 0;
  BlobIDType blobID = ++m_nextBlobID;
  
  while (insertedSize  < value.size()) {
    // no need to init new page, cuz the new page has been inited by m_dataFileManager .
    Page *rawNewBlobPage = m_dataFileManager->newBlobPage();
    rawNewBlobPage->wLatch();
    auto newBlobPage = reinterpret_cast<BlobFilePage *>(rawNewBlobPage->getData());
    newBlobPage->setBlobID(blobID);
    newBlobPage->setIsBlobListHead(false);
    
    size_t sizeToInsert;
    if (prevPage == nullptr) {
      firstPageID = rawNewBlobPage->getPageID();
      
      auto newBlobHeadPage = reinterpret_cast<BlobHeadPage *>(newBlobPage);
      newBlobHeadPage->setKey(key);
      sizeToInsert = std::min(value.size() - insertedSize, BlobHeadPage::MAX_DATA_SIZE);
      newBlobHeadPage->writeData(value.c_str() + insertedSize, sizeToInsert);
    } else {
      reinterpret_cast<BlobFilePage *>(prevPage->getData())->setBlobListNextPageID(rawNewBlobPage->getPageID());
      prevPage->wUnlatch();
      m_dataFileManager->unpinBlobPage(prevPage->getPageID(), true);
  
      auto newNonHeadPage = reinterpret_cast<BlobNonHeadPage *>(newBlobPage);
      sizeToInsert = std::min(value.size() - insertedSize, BlobNonHeadPage::MAX_DATA_SIZE);
      newNonHeadPage->writeData(value.c_str() + insertedSize, sizeToInsert);
    }
    
    insertedSize += sizeToInsert;
    prevPage = rawNewBlobPage;
  }
  
  prevPage->wUnlatch();
  m_dataFileManager->unpinBlobPage(prevPage->getPageID(), true);
  
  Page *rawBlobHeadPage = m_dataFileManager->fetchBlobPage(firstPageID);
  auto blobHeadPage = reinterpret_cast<BlobFilePage *>
      (rawBlobHeadPage->getData());
  blobHeadPage->setIsBlobListHead(true);
  rawBlobHeadPage->wUnlatch();
  m_dataFileManager->unpinBlobPage(firstPageID, true);
  
  return std::string(reinterpret_cast<const char *>(&firstPageID), sizeof(firstPageID));
}

std::optional<std::string>
DataAccessor::getBlob(const std::string& key, const std::string &blobFirstPageID) const {
  PageIDType nextBlobPageID = *reinterpret_cast<const PageIDType *>(blobFirstPageID.c_str());
  bool isHeadPage = true;
  BlobIDType blobID;
  std::string blob;

  while (nextBlobPageID != INVALID_PAGE_ID) {
    Page *rawBlobPage = m_dataFileManager->fetchBlobPage(nextBlobPageID);
    rawBlobPage->rLatch();
    if (reinterpret_cast<BlobFileFreePage *>(rawBlobPage->getData())->isFreePage()) { // if try to read a free page
      rawBlobPage->rUnlatch();
      m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
      return std::nullopt;
    }
    
    auto blobPage = reinterpret_cast<BlobFilePage *>(rawBlobPage->getData());
    if (isHeadPage) {
      isHeadPage = false;
      if (!blobPage->isBlobListHead()) {
        rawBlobPage->rUnlatch();
        m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
        return std::nullopt;
      }

      auto blobHeadPage = reinterpret_cast<BlobHeadPage *>(blobPage);
      auto forTest = blobHeadPage->getKey();
      if (blobHeadPage->getKey() != key) {
        rawBlobPage->rUnlatch();
        m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
        return std::nullopt;
      }
      
      blobID = blobHeadPage->getBlobID();
      blob.append(blobHeadPage->readData());
    } else {
      if (blobPage->isBlobListHead()) {
        rawBlobPage->rUnlatch();
        m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
        return std::nullopt;
      }
  
      auto blobNonHeadPage = reinterpret_cast<BlobNonHeadPage *>(blobPage);
      if (blobNonHeadPage->getBlobID() != blobID) {
        rawBlobPage->rUnlatch();
        m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
        return std::nullopt;
      }
      
      blob.append(blobNonHeadPage->readData());
    }
    
    nextBlobPageID = blobPage->getBlobListNextPageID();
    rawBlobPage->rUnlatch();
    m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
  }
  return blob;
}

void DataAccessor::removeBlob(const std::string &blobFirstPageID) {
  PageIDType nextBlobPageID = *reinterpret_cast<const PageIDType *>(blobFirstPageID.c_str());
  while (nextBlobPageID != INVALID_PAGE_ID) {
    Page *rawBlobPage = m_dataFileManager->fetchBlobPage(nextBlobPageID);
    rawBlobPage->rLatch();
    auto blobPage = reinterpret_cast<BlobFilePage *>(rawBlobPage->getData());
    PageIDType thisBlobPageID = nextBlobPageID;
    nextBlobPageID = blobPage->getBlobListNextPageID();
    rawBlobPage->rUnlatch();
    m_dataFileManager->unpinBlobPage(thisBlobPageID, false);
    m_dataFileManager->deleteBlobPage(thisBlobPageID);
  }
}
}
