#include "buffer/buffer_pool_manager.h"

namespace MisakiDB{
BufferPoolManager::BufferPoolManager(size_t poolSize, FileStore *fileStore, bool enableCondVar)
    : m_poolSize(poolSize), m_fileStore(fileStore), m_enableCondVar(enableCondVar) {
  // We allocate a consecutive memory space for the buffer pool.
  m_pages = new Page[m_poolSize];
  m_replacer = new LRUReplacer;
  
  // Initially, every page is in the free list.
  for (size_t i = 0; i < m_poolSize; ++i) {
    m_freeList.emplace_back(static_cast<FrameIDType>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] m_pages;
  delete m_replacer;
}

Page *BufferPoolManager::fetchExistentPage(FileType fileType, PageIDType pageID) {
  FrameIDType frameID;
  Page *p;
  
  // find the <<fileType, pageID>, frame_id> entry
  auto entryIter = m_pageTable.find({fileType, pageID});
  
  // if entry exist
  if (entryIter != m_pageTable.end()) {
    frameID = entryIter->second;
    p = m_pages + frameID;
    return p;
  }
  
  // if no page found
  return nullptr;
}

Page *BufferPoolManager::getVictimPage() {
  FrameIDType frameID;
  Page *p;
  
  if (!m_freeList.empty()) { // if there are free frames,
    frameID = m_freeList.front();
    m_freeList.pop_front();
    p = m_pages + frameID;
    
    return p;
  }
  
  if (m_replacer->victim(&frameID)) { // if no free frames, try to evict a page
    p = m_pages + frameID;
    m_pageTable.erase({p->m_fileType, p->m_pageID});
    
    // write to disk if the page is dirty
    if (p->isDirty()) {
      m_fileStore->writeRawPage(p->m_fileType, p->m_pageID, p->m_data);
      p->m_isDirty = false;
    }
    
    return p;
  }
  
  // if failing to find victim, return nullptr
  return nullptr;
}

bool BufferPoolManager::flushPage_helper(FileType fileType, PageIDType pageID) {
  assert(fileType != FileType::INVALID || pageID != INVALID_PAGE_ID);
  
  Page *p = fetchExistentPage(fileType, pageID);
  if (p == nullptr) {
    return false;
  }
  
  // flush a page back to disk no matter how its dirty bit set,
  // cuz the test will write back the page directly by flush
  // without unpinning the page first
  m_fileStore->writeRawPage(fileType, pageID, p->m_data);
  p->m_isDirty = false;
  
  return true;
}

Page *BufferPoolManager::fetchPage(FileType fileType, PageIDType pageID) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::unique_lock<std::mutex> lck(m_poolLatch);
  Page *p = fetchExistentPage(fileType, pageID);
  
  if (p != nullptr) {
    if (p->m_pinCount == 0) {
      m_replacer->pin(p - m_pages);
    }
    ++p->m_pinCount;
    return p;
  }

  if (m_enableCondVar) {
    this->m_poolCondition.wait(lck, [&] { return (p = getVictimPage()) != nullptr; });
  } else {
    p = getVictimPage();
  }

  if (p != nullptr) {
    ++p->m_pinCount;
    m_pageTable[{fileType, pageID}] = (p - m_pages);  // add an entry to page table(p - m_pages is to get the frame id)
    p->m_fileType = fileType;
    p->m_pageID = pageID;
    m_fileStore->readRawPage(p->m_fileType, p->m_pageID, p->m_data);
    return p;
  }

  return nullptr;
}

bool BufferPoolManager::unpinPage(FileType fileType, PageIDType pageID, bool isDirty) {
  std::lock_guard<std::mutex> lck(m_poolLatch);
  Page *p = fetchExistentPage(fileType, pageID);
  
  // if page does not exist in buffer pool
  if (p == nullptr) {
    return true;
  }
  
  if (!p->m_isDirty && isDirty) {
    p->m_isDirty = isDirty;
  }
  
  if (p->m_pinCount <= 0) {
    return false;
  }
  
  if (--p->m_pinCount == 0) {
    m_replacer->unpin(p - m_pages);
    if (m_enableCondVar) {
      this->m_poolCondition.notify_one();
    }
  }
  
  return true;
}

bool BufferPoolManager::flushPage(FileType fileType, PageIDType pageID) {
  assert(fileType != FileType::INVALID || pageID != INVALID_PAGE_ID);
  std::lock_guard<std::mutex> lck(m_poolLatch);
  return flushPage_helper(fileType, pageID);
}

void BufferPoolManager::flushAllPages() {
  std::lock_guard<std::mutex> lck(m_poolLatch);
  for (auto entry : m_pageTable) {
    FileType fileType = entry.first.first;
    PageIDType pageID = entry.first.second;
    flushPage_helper(fileType, pageID);
  }
}

Page *BufferPoolManager::appendNewPage(FileType fileType, PageIDType pageID) {
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Return a pointer to P.
  std::unique_lock<std::mutex> lck(m_poolLatch);
  Page *p {  };

  if (m_enableCondVar) {
    this->m_poolCondition.wait(lck, [&] { return (p = getVictimPage()) != nullptr; });
  } else {
    p = getVictimPage();
  }

  if (p != nullptr) {
    ++p->m_pinCount;
    m_pageTable[{fileType, pageID}] = (p - m_pages);  // add an entry to page table(p - m_pages is to get the frame id)
    p->m_fileType = fileType;
    p->m_pageID = pageID;
    p->resetMemory();
    
    // make sure new page is written back to disk
    flushPage_helper(fileType, pageID);
    return p;
  }

  return nullptr;
}
}
