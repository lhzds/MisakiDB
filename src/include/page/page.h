#pragma once
#include "globals.h"

namespace MisakiDB {
/**
 * Page is the basic unit of storage within the database system. Page provides a wrapper for actual data pages being
 * held in main memory. Page also contains book-keeping information that is used by the buffer pool manager, e.g.
 * pin count, dirty flag, page id, etc.
 */
class Page {
  // There is book-keeping information inside the page that should only be relevant to the buffer pool manager.
  friend class BufferPoolManager;

public:
  /** Constructor. Zeros out the page data. */
  Page() { resetMemory(); }
  
  /** Default destructor. */
  ~Page() = default;
  
  /** @return the actual data contained within this page */
  inline char *getData() { return m_data; }
  
  /** @return the page id of this page */
  inline PageIDType getPageId() const { return m_pageID; }
  
  /** @return the pin count of this page */
  inline int getPinCount() const { return m_pinCount; }
  
  /** @return true if the page in memory has been modified from the page on disk, false otherwise */
  inline bool isDirty() const { return m_isDirty; }

protected:
  static constexpr size_t OFFSET_PAGE_START = 0;

private:
  /** Zeroes out the data that is held within the page. */
  inline void resetMemory() { memset(m_data, OFFSET_PAGE_START, PAGE_SIZE); }
  
  /** The type of file which this page belongs. */
  FILE_TYPE m_fileType = ;
  /** The actual data that is stored within a page. */
  char m_data[PAGE_SIZE]{};
  /** The ID of this page. */
  PageIDType m_pageID = INVALID_PAGE_ID ;
  /** The pin count of this page. */
  int m_pinCount = 0;
  /** True if the page is dirty, i.e. it is different from its corresponding page on disk. */
  bool m_isDirty = false;
};
}

