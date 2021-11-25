#pragma once
#include "globals.h"
#include "buffer/index_buffer_pool_manager.h"
#include "page/index_file_free_page.h"

namespace MisakiDB{
class IndexFileManager {
public:
  explicit IndexFileManager(IndexBufferPoolManager *indexBufferPoolManager);
  
  Page *fetchIndexPage(PageIDType pageID);
  bool unpinIndexPage(PageIDType pageID, bool isDirty);
  
  Page *newIndexPage();
  bool deleteIndexPage(PageIDType pageID);
  
private:
  IndexBufferPoolManager *m_indexBufferPoolManager;
};
}
