#include "buffer/index_buffer_pool_manager.h"

namespace MisakiDB {

IndexBufferPoolManager::IndexBufferPoolManager(size_t poolSize, FileStore *fileStore)
    : BufferPoolManager { poolSize, fileStore } { }

Page *IndexBufferPoolManager::fetchIndexPage(PageIDType pageID) {
  return BufferPoolManager::fetchPage(FILE_TYPE::INDEX, pageID, true);
}

bool IndexBufferPoolManager::unpinIndexPage(PageIDType pageID, bool isDirty) {
  return BufferPoolManager::unpinPage(FILE_TYPE::INDEX, pageID, isDirty, true);
}

bool IndexBufferPoolManager::flushIndexPage(PageIDType pageID) {
  return BufferPoolManager::flushPage(FILE_TYPE::INDEX, pageID);
}

Page *IndexBufferPoolManager::appendNewIndexPage(PageIDType pageID) {
  return BufferPoolManager::appendNewPage(FILE_TYPE::INDEX, pageID, true);
}
}
