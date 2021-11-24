#include "file_manager/data_file_manager.h"
#include "page/free_space_map_file_page.h"
#include "page/data_file_page.h"
#include "page/data_file_header.h"

namespace MisakiDB {
DataFileManager::DataFileManager(DataBufferPoolManager *dataBufferPoolManager)
    :m_dataBufferPoolManager(dataBufferPoolManager) {}

Page *DataFileManager::fetchDataPage(PageIDType pageID) {
  return m_dataBufferPoolManager->fetchDataPage(pageID);
}

bool DataFileManager::unpinDataPage(PageIDType pageID, bool isDirty) {
  return m_dataBufferPoolManager->unpinDataPage(pageID, isDirty);
}

bool DataFileManager::flushDataPage(PageIDType pageID) {
  return m_dataBufferPoolManager->flushDataPage(pageID);
}

Page *DataFileManager::allocateDataPage(RecordSizeType recordSize) {
  Page *rawMapHeader = m_dataBufferPoolManager->fetchFreeSpaceMapPage(0);
  rawMapHeader->wLatch();
  auto mapHeader = reinterpret_cast<FreeSpaceMapFileHeader *>(rawMapHeader->getData());
  
  PageIDType dataPageID = INVALID_PAGE_ID;
  bool found = false;
  for (PageIDType i = 1; !found && i < mapHeader->getNextPageID(); ++i) {
    auto mapPage = reinterpret_cast<FreeSpaceMapFilePage *>
      (m_dataBufferPoolManager->fetchFreeSpaceMapPage(i)->getData());
    
    for (int j = 0; !found && j < FreeSpaceMapFilePage::MAX_SIZE; ++j) {
      if (mapPage->getFreeSpace(j) >= recordSize + DataFilePage::SLOT_SIZE) {
        found = true;
        mapPage->subFreeSpace(j, recordSize + DataFilePage::SLOT_SIZE);
        
        dataPageID = convertToDataPageID(i, j);
      }
    }
    
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(i, false);
  }
  
  if (!found) { // if fail to find a fit data page from existing free space map page
    // create and init new free space map file page
    Page *rawNewMapPage = m_dataBufferPoolManager->appendNewFreeSpaceMapPage(mapHeader->getNextPageID());
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(rawNewMapPage->getPageID(), false);
    auto newMapPage = reinterpret_cast<FreeSpaceMapFilePage *>(rawNewMapPage->getData());
    newMapPage->init();
    newMapPage->subFreeSpace(0, recordSize + DataFilePage::SLOT_SIZE);

    dataPageID = convertToDataPageID(mapHeader->getNextPageID(), 0);
    mapHeader->incNextPageID();
    
    rawMapHeader->wUnlatch();
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(0, true);
  } else {
    rawMapHeader->wUnlatch();
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(0, false);
  }
  
  Page *rawDataPage;
  Page *rawDataHeader = m_dataBufferPoolManager->fetchDataPage(0);
  rawDataHeader->wLatch();
  auto dataHeader = reinterpret_cast<DataFileHeader *>(rawDataHeader->getData());
  if (dataHeader->getNextPageID() <= dataPageID) { // if the data file page do not yet exist
    // create and init new data file page
    rawDataPage = m_dataBufferPoolManager->appendNewDataPage(dataPageID);
    auto dataPage = reinterpret_cast<DataFilePage *>(rawDataPage->getData());
    dataPage->init();
    
    dataHeader->incNextPageID();
    rawDataHeader->wUnlatch();
    m_dataBufferPoolManager->unpinDataPage(0, true);
  } else {
    rawDataPage = m_dataBufferPoolManager->fetchDataPage(dataPageID);
    rawDataHeader->wUnlatch();
    m_dataBufferPoolManager->unpinDataPage(0, false);
  }
  
  return rawDataPage;
}

void DataFileManager::addFreeSpace(PageIDType pageID, RecordSizeType recordSize) {
  PageIDType mapPageID = convertToMapPageID(pageID);
  int mapIndex = convertToMapIndex(pageID);
  Page *rawMapPage = m_dataBufferPoolManager->fetchFreeSpaceMapPage(mapPageID);
  rawMapPage->wLatch();
  auto mapPage = reinterpret_cast<FreeSpaceMapFilePage *>(rawMapPage->getData());
  mapPage->addFreeSpace(mapIndex, recordSize + DataFilePage::SLOT_SIZE);
  rawMapPage->wUnlatch();
  m_dataBufferPoolManager->unpinFreeSpaceMapPage(mapPageID, true);
}

PageIDType DataFileManager::convertToDataPageID(PageIDType mapPageID, int mapIndex) {
  return (mapPageID - 1) * FreeSpaceMapFilePage::MAX_SIZE + mapIndex + 1; // dataPageID start at 1
}

PageIDType DataFileManager::convertToMapPageID(PageIDType dataPageID) {
  return (dataPageID - 1) / FreeSpaceMapFilePage::MAX_SIZE + 1;
}

int DataFileManager::convertToMapIndex(PageIDType dataPageID) {
  return (dataPageID - 1) % FreeSpaceMapFilePage::MAX_SIZE;
}

}
