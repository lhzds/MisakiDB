#include "file_manager/index_file_manager.h"
#include "page/index_file_header.h"


namespace MisakiDB {
IndexFileManager::IndexFileManager(IndexBufferPoolManager *indexBufferPoolManager)
    :m_indexBufferPoolManager(indexBufferPoolManager) {}

Page *IndexFileManager::fetchIndexPage(PageIDType pageID) {
  return m_indexBufferPoolManager->fetchIndexPage(pageID);
}

bool IndexFileManager::unpinIndexPage(PageIDType pageID, bool isDirty) {
  return m_indexBufferPoolManager->unpinIndexPage(pageID, isDirty);
}

bool IndexFileManager::flushIndexPage(PageIDType pageID) {
  return m_indexBufferPoolManager->flushIndexPage(pageID);
}

Page *IndexFileManager::newPage(PageIDType *pageID) {
  std::lock_guard<std::mutex> lck {m_fileLatch};
  
  auto header = reinterpret_cast<IndexFileHeader *>(m_indexBufferPoolManager->fetchIndexPage(0)->getData());
  Page *newPage;
  if (header->getFreePageListHeader() == INVALID_PAGE_ID) {
    newPage = m_indexBufferPoolManager->appendNewIndexPage(header->getNextPageID());
    header->incNextPageID();
  } else {
    newPage = m_indexBufferPoolManager->fetchIndexPage(header->getFreePageListHeader());
    auto freePage = reinterpret_cast<IndexFileFreePage *>(newPage->getData());
    header->setFreePageListHeader(freePage->getNextFreePageID());
    // zero out the data
    memset(newPage->getData(), 0, PAGE_SIZE);
  }
  
  m_indexBufferPoolManager->unpinIndexPage(0, true);
  if (pageID != nullptr) {
    *pageID = newPage->getPageID();
  }
  return newPage;
}

bool IndexFileManager::deletePage(PageIDType pageID) {
  std::lock_guard<std::mutex> lck {m_fileLatch};
  auto header = reinterpret_cast<IndexFileHeader *>(m_indexBufferPoolManager->fetchIndexPage(0)->getData());
  if (pageID == INVALID_PAGE_ID || pageID < 1 || header->getNextPageID() <= pageID) { // invalid page id
    return false;
  }
  
  Page *page = m_indexBufferPoolManager->fetchIndexPage(pageID);
  if (page->getPinCount() != 1) {
    return false;
  }
  auto freePage = reinterpret_cast<IndexFileFreePage *>(page->getData());
  if (freePage ->isFreePage()) {     // page have been deleted
    return false;
  }
  freePage ->init();
  freePage ->setNextFreePageID(header->getFreePageListHeader());
  header->setFreePageListHeader(pageID);
  m_indexBufferPoolManager->unpinIndexPage(pageID, true);
  m_indexBufferPoolManager->unpinIndexPage(0, true);
  return true;
}
}