#include "data_accessor.h"
#include "page/data_file_page.h"

namespace MisakiDB{
DataAccessor::DataAccessor(DataFileManager *dataFileManager)
    :m_dataFileManager(dataFileManager) {}

std::string DataAccessor::getData(RecordIDType recordID) const {
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->rLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  std::string record = targetPage->getRecord(slotArrayIndex);
  rawTargetPage->rUnlatch();
  m_dataFileManager->unpinDataPage(pageID, false);
  return record;
}

void DataAccessor::removeData(RecordIDType recordID) {
  PageIDType pageID { recordID.pageID };
  int slotArrayIndex { recordID.slotArrayIndex };
  
  Page *rawTargetPage = m_dataFileManager->fetchDataPage(pageID);
  rawTargetPage->wLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  auto removedRecordSize = targetPage->removeRecord(slotArrayIndex);
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(pageID, true);
  m_dataFileManager->addFreeSpace(pageID, removedRecordSize);
}

RecordIDType DataAccessor::insertData(const std::string &value) {
  Page *rawTargetPage = m_dataFileManager->allocateDataPage(value.size());
  rawTargetPage->wLatch();
  auto targetPage = reinterpret_cast<DataFilePage *>(rawTargetPage->getData());
  int slotArrayIndex = targetPage->insertRecord(value);
  RecordIDType newRecordID;
  newRecordID.pageID = rawTargetPage->getPageID();
  newRecordID.slotArrayIndex = slotArrayIndex;
  rawTargetPage->wUnlatch();
  m_dataFileManager->unpinDataPage(newRecordID.pageID, true);
  return newRecordID;
}

// TODO : if > MAX_RECORD_SIZE
}
