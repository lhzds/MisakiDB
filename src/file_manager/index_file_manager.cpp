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

Page *IndexFileManager::newIndexPage() {
  Page *rawHeader = m_indexBufferPoolManager->fetchIndexPage(0);
  auto header = reinterpret_cast<IndexFileHeader *>(rawHeader->getData());
  Page *rawNewPage;
  if (header->getFreePageListHeader() == INVALID_PAGE_ID) {
    rawNewPage = m_indexBufferPoolManager->appendNewIndexPage(header->getNextPageID());
    header->incNextPageID();
  } else {
    rawNewPage = m_indexBufferPoolManager->fetchIndexPage(header->getFreePageListHeader());
    auto newPage = reinterpret_cast<IndexFileFreePage *>(rawNewPage->getData());
    header->setFreePageListHeader(newPage->getNextFreePageID());
    // zero out the data
    memset(rawNewPage->getData(), 0, PAGE_SIZE);
  }
  m_indexBufferPoolManager->unpinIndexPage(0, true);
  return rawNewPage;
}

bool IndexFileManager::deleteIndexPage(PageIDType pageID) {
  Page *rawHeader = m_indexBufferPoolManager->fetchIndexPage(0);
  auto header = reinterpret_cast<IndexFileHeader *>(rawHeader->getData());
  if (pageID == INVALID_PAGE_ID || pageID < 1 || header->getNextPageID() <= pageID) { // invalid page id(Header page can't be deleted)
    m_indexBufferPoolManager->unpinIndexPage(0, false);
    return false;
  }
  
  Page *rawFreePage = m_indexBufferPoolManager->fetchIndexPage(pageID);
  if (rawFreePage->getPinCount() != 1) {
    m_indexBufferPoolManager->unpinIndexPage(pageID, false);
    m_indexBufferPoolManager->unpinIndexPage(0, false);
    return false;
  }
  
  auto freePage = reinterpret_cast<IndexFileFreePage *>(rawFreePage->getData());
  if (freePage ->isFreePage()) {     // page have been deleted
    m_indexBufferPoolManager->unpinIndexPage(pageID, false);
    m_indexBufferPoolManager->unpinIndexPage(0, false);
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