#include "buffer/data_buffer_pool_manager.h"

namespace MisakiDB {
DataBufferPoolManager::DataBufferPoolManager(size_t poolSize, FileStore *fileStore)
    : BufferPoolManager { poolSize, fileStore, true } { }

Page *DataBufferPoolManager::fetchDataPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FILE_TYPE::DATA, pageID);
}

bool DataBufferPoolManager::unpinDataPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FILE_TYPE::DATA, pageID, isDirty);
}

bool DataBufferPoolManager::flushDataPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FILE_TYPE::DATA, pageID);
}

Page *DataBufferPoolManager::appendNewDataPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FILE_TYPE::DATA, pageID);
}

Page *DataBufferPoolManager::fetchFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FILE_TYPE::FREE_SPACE_MAP, pageID);
}

bool DataBufferPoolManager::unpinFreeSpaceMapPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FILE_TYPE::FREE_SPACE_MAP, pageID, isDirty);
}

bool DataBufferPoolManager::flushFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FILE_TYPE::FREE_SPACE_MAP, pageID);
}

Page *DataBufferPoolManager::appendNewFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FILE_TYPE::FREE_SPACE_MAP, pageID);
}
}
