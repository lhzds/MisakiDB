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
  auto mapHeader = reinterpret_cast<FreeSpaceMapFileHeader *>
      (m_dataBufferPoolManager->fetchFreeSpaceMapPage(0)->getData());
  
  PageIDType dataPageID = INVALID_PAGE_ID;
  bool found = false;
  for (PageIDType i = 1; !found && i < mapHeader->getNextPageID(); ++i) {
    auto mapPage = reinterpret_cast<FreeSpaceMapFilePage *>
      (m_dataBufferPoolManager->fetchFreeSpaceMapPage(i)->getData());
    
    for (int j = 0; !found && j < FreeSpaceMapFilePage::MAX_SIZE; ++j) {
      if (mapPage->getFreeSpace(j) >= recordSize + DataFilePage::SLOT_SIZE) {
        found = true;
        dataPageID = convertToDataPageID(i, j);
      }
    }
    
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(i, false);
  }
  
  if (!found) {
    Page *newMapPage = m_dataBufferPoolManager->appendNewFreeSpaceMapPage(mapHeader->getNextPageID());
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(newMapPage->getPageID(), false);
    auto mapFilePage = reinterpret_cast<FreeSpaceMapFilePage *>(newMapPage->getData());
    mapFilePage->init();
    
    dataPageID = convertToDataPageID(mapHeader->getNextPageID(), 0);
    mapHeader->incNextPageID();
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(0, true);
  } else {
    m_dataBufferPoolManager->unpinFreeSpaceMapPage(0, false);
  }
  
  Page *newDataPage;
  auto dataFileHeader = reinterpret_cast<DataFileHeader *>
    (m_dataBufferPoolManager->fetchDataPage(0)->getData());
  if (dataFileHeader->getNextPageID() <= dataPageID) {
    dataFileHeader->incNextPageID();
    m_dataBufferPoolManager->unpinDataPage(0, true);
    
    newDataPage = m_dataBufferPoolManager->appendNewDataPage(dataPageID);
    auto dataFilePage = reinterpret_cast<DataFilePage *>(newDataPage->getData());
    dataFilePage->init();
  } else {
    newDataPage = m_dataBufferPoolManager->fetchDataPage(dataPageID);
    m_dataBufferPoolManager->unpinDataPage(0, false);
  }
  
  return newDataPage;
}

void DataFileManager::subFreeSpace(PageIDType pageID, RecordSizeType recordSize) {
  PageIDType mapPageID = convertToMapPageID(pageID);
  int mapIndex = convertToMapIndex(pageID);
  auto mapPage = reinterpret_cast<FreeSpaceMapFilePage *>
    (m_dataBufferPoolManager->fetchFreeSpaceMapPage(mapPageID)->getData());
  mapPage->subFreeSpace(mapIndex, recordSize + DataFilePage::SLOT_SIZE);
  m_dataBufferPoolManager->unpinFreeSpaceMapPage(mapPageID, true);
}

void DataFileManager::addFreeSpace(PageIDType pageID, RecordSizeType recordSize) {
  PageIDType mapPageID = convertToMapPageID(pageID);
  int mapIndex = convertToMapIndex(pageID);
  auto mapPage = reinterpret_cast<FreeSpaceMapFilePage *>
    (m_dataBufferPoolManager->fetchFreeSpaceMapPage(mapPageID)->getData());
  mapPage->addFreeSpace(mapIndex, recordSize + DataFilePage::SLOT_SIZE);
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
