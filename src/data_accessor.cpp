#include "data_accessor.h"
#include "page/data_file_page.h"
#include "page/blob_file_page.h"
#include "globals.h"

namespace MisakiDB{
DataAccessor::DataAccessor(DataFileManager *dataFileManager)
    :m_dataFileManager(dataFileManager) {}

std::string DataAccessor::getData(RecordIDType recordID) const {
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->rLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  auto [record, isBlob] = targetPage->getRecord(slotArrayIndex);
  rawTargetPage->rUnlatch();
  m_dataFileManager->unpinDataPage(pageID, false);
  if (isBlob) {
    return getBlob(record);
  }
  return record;
}

void DataAccessor::removeData(RecordIDType recordID) {
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->wLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  auto removeResult = targetPage->removeRecord(slotArrayIndex);
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(pageID, true);
  if (removeResult.index() == 0) {
    m_dataFileManager->addFreeSpace(pageID, std::get<RecordSizeType>(removeResult));
  } else {
    std::string blobFirstPageID = std::move(std::get<std::string>(removeResult));
    m_dataFileManager->addFreeSpace(pageID, blobFirstPageID.size());
    removeBlob(blobFirstPageID);
  }
}

RecordIDType DataAccessor::insertData(const std::string &value) {
  Page *rawTargetPage;
  int slotArrayIndex;
  if (DataFilePage::MAX_RECORD_SIZE < value.size()) { // if value is blob
    std::string blobFirstPageID = insertBlob(value);
    rawTargetPage = m_dataFileManager->allocateDataPage(blobFirstPageID.size());
    rawTargetPage->wLatch();
    auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
    slotArrayIndex = targetPage->insertRecord(blobFirstPageID, true);
  } else {
    rawTargetPage = m_dataFileManager->allocateDataPage(value.size());
    rawTargetPage->wLatch();
    auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
    slotArrayIndex = targetPage->insertRecord(value, false);
  }
  
  RecordIDType newRecordID;
  newRecordID.pageID = rawTargetPage->getPageID();
  newRecordID.slotArrayIndex = slotArrayIndex;
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(newRecordID.pageID, true);
  return newRecordID;
}

std::string DataAccessor::insertBlob(const std::string &value) {
  Page *prevPage = nullptr;
  PageIDType firstPageID;
  size_t insertedSize = 0;
  while (insertedSize  < value.size()) {
    // 1. No need to latch the new page, cuz no one else has chance to access it.
    // 2. no need to init new page, cuz the new page has been inited by m_dataFileManager .
    Page *rawNewBlobPage = m_dataFileManager->newBlobPage();
    
    if (prevPage == nullptr) {
      firstPageID = rawNewBlobPage->getPageID();
    } else {
      reinterpret_cast<BlobFilePage *>(prevPage->getData())->setBlobListNextPageID(rawNewBlobPage->getPageID());
      m_dataFileManager->unpinBlobPage(prevPage->getPageID(), true);
    }
    
    auto newBlobPage = reinterpret_cast<BlobFilePage *>(rawNewBlobPage->getData());
    size_t sizeToInsert = std::min(value.size() - insertedSize, BlobFilePage::MAX_DATA_SIZE);
    newBlobPage->writeData(value.c_str() + insertedSize, sizeToInsert);
    insertedSize += sizeToInsert;
    prevPage = rawNewBlobPage;
  }
  m_dataFileManager->unpinBlobPage(prevPage->getPageID(), true);
  return std::to_string(firstPageID);
}

std::string DataAccessor::getBlob(const std::string &blobFirstPageID) const {
  PageIDType nextBlobPageID = std::stoul(blobFirstPageID);
  std::string blob;
  while (nextBlobPageID != INVALID_PAGE_ID) {
    Page *rawBlobPage = m_dataFileManager->fetchBlobPage(nextBlobPageID);
    rawBlobPage->rLatch();
    auto blobPage = reinterpret_cast<BlobFilePage *>(rawBlobPage->getData());
    blob.append(blobPage->readData());
    nextBlobPageID = blobPage->getBlobListNextPageID();
    rawBlobPage->rUnlatch();
    m_dataFileManager->unpinBlobPage(rawBlobPage->getPageID(), false);
  }
  return blob;
}

void DataAccessor::removeBlob(const std::string &blobFirstPageID) {
  PageIDType nextBlobPageID = std::stoul(blobFirstPageID);
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
