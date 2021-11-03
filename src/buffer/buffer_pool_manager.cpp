#include "buffer/buffer_pool_manager.h"

namespace MisakiDB{
BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  m_pages = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);
  
  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] m_pages;
  delete replacer_;
}

Page *BufferPoolManager::FetchExistentPage(page_id_t page_id) {
  frame_id_t frame_id;
  Page *p;
  
  // find the <page_id, frame_id> entry
  auto entry_iter = page_table_.find(page_id);
  
  // if entry exist
  if (entry_iter != page_table_.end()) {
    frame_id = entry_iter->second;
    p = m_pages + frame_id;
    return p;
  }
  
  return nullptr;
}

Page *BufferPoolManager::GetVictimPage() {
  frame_id_t frame_id;
  Page *p;
  
  if (!free_list_.empty()) { // if there are free frames,
    frame_id = free_list_.front();
    free_list_.pop_front();
    p = m_pages + frame_id;
    
    return p;
  } else if (replacer_->Victim(&frame_id)) { // if no free frames, try to evist a page
    p = m_pages + frame_id;
    page_table_.erase(p->page_id_);  // NOLINT
    
    // write to disk if the page is dirty
    if (p->IsDirty()) {  // NOLINT
      m_fileStore->WritePage(p->page_id_, p->data_);
      p->is_dirty_ = false;
    }
    
    return p;
  }
  
  // if failing to find victim, return nullptr
  return nullptr;
}

bool BufferPoolManager::FlushPageHelper(page_id_t page_id) {
  assert(page_id != INVALID_PAGE_ID);
  
  Page *p = FetchExistentPage(page_id);
  if (p == nullptr) {
    return false;
  }
  
  // flush a page back to disk no matter how its dirty bit set,
  // cuz the test will write back the page directly by flush
  // without unpinning the page first
  m_fileStore->WritePage(page_id, p->data_);
  p->is_dirty_ = false;
  
  return true;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  std::lock_guard<std::mutex> lck(latch_);
  Page *p = FetchExistentPage(page_id);
  
  if (p != nullptr) {
    if (p->pin_count_ == 0) {
      replacer_->Pin(p - m_pages);
    }
    ++p->pin_count_;
  } else {
    p = GetVictimPage();
    if (p != nullptr) {
      ++p->pin_count_;
      page_table_[page_id] = (p - m_pages);  // add a entry to page table(p - m_pages is to get the frame id)
      p->page_id_ = page_id;
      m_fileStore->ReadPage(p->page_id_, p->data_);
    }
  }
  return p;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  std::lock_guard<std::mutex> lck(latch_);
  Page *p = FetchExistentPage(page_id);
  
  // if page does not exist in buffer pool
  if (p == nullptr) {
    return true;
  }
  
  if (!p->is_dirty_ && is_dirty) {
    p->is_dirty_ = is_dirty;
  }
  
  if (p->pin_count_ <= 0) {
    return false;
  }
  
  if (--p->pin_count_ == 0) {
    replacer_->Unpin(p - m_pages);
  }
  
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  assert(page_id != INVALID_PAGE_ID);
  std::lock_guard<std::mutex> lck(latch_);
  
  return FlushPageHelper(page_id);
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  std::lock_guard<std::mutex> lck(latch_);
  
  Page *p = GetVictimPage();
  if (p != nullptr) {
    *page_id = m_fileStore->AllocatePage();
    ++p->pin_count_;
    page_table_[*page_id] = (p - m_pages);  // add a entry to page table(p - m_pages is to get the frame id)
    p->page_id_ = *page_id;
    
    // make sure new page is written back to disk
    FlushPageHelper(*page_id);
    
    memset(p->data_, 0, sizeof(p->data_));
    return p;
  }
  
  return nullptr;
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  std::lock_guard<std::mutex> lck(latch_);
  Page *p = FetchExistentPage(page_id);
  if (p == nullptr) {
    m_fileStore->DeallocatePage(page_id);
    return true;
  }
  
  // can't delete a referenced page
  if (p->pin_count_ != 0) {
    return false;
  }
  
  m_fileStore->DeallocatePage(page_id);
  // reset frame to free state
  p->is_dirty_ = false;
  p->page_id_ = INVALID_PAGE_ID;
  page_table_.erase(page_id);
  
  // without this line, the same frame_id will have a copy both in free_list and replacer
  replacer_->Pin(p - m_pages);
  
  free_list_.push_back(p - m_pages);
  return true;
}

void BufferPoolManager::FlushAllPagesImpl() {
  std::lock_guard<std::mutex> lck(latch_);
  for (auto entry : page_table_) {
    FlushPageHelper(entry.second);
  }
}
}
