#include "buffer/data_buffer_pool_manager.h"

namespace MisakiDB {
DataBufferPoolManager::DataBufferPoolManager(size_t poolSize, FileStore *fileStore)
    : BufferPoolManager { poolSize, fileStore, true } { }

Page *DataBufferPoolManager::fetchDataPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FileType::DATA, pageID);
}

bool DataBufferPoolManager::unpinDataPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FileType::DATA, pageID, isDirty);
}

bool DataBufferPoolManager::flushDataPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FileType::DATA, pageID);
}

Page *DataBufferPoolManager::appendNewDataPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FileType::DATA, pageID);
}

Page *DataBufferPoolManager::fetchFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FileType::FREE_SPACE_MAP, pageID);
}

bool DataBufferPoolManager::unpinFreeSpaceMapPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FileType::FREE_SPACE_MAP, pageID, isDirty);
}

bool DataBufferPoolManager::flushFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FileType::FREE_SPACE_MAP, pageID);
}

Page *DataBufferPoolManager::appendNewFreeSpaceMapPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FileType::FREE_SPACE_MAP, pageID);
}

Page *DataBufferPoolManager::fetchBlobPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FileType::BLOB, pageID);
}

bool DataBufferPoolManager::unpinBlobPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FileType::BLOB, pageID, isDirty);
}

bool DataBufferPoolManager::flushBlobPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FileType::BLOB, pageID);
}

Page *DataBufferPoolManager::appendNewBlobPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FileType::BLOB, pageID);
}

}
