#pragma once
#include "globals.h"
#include "buffer_pool_manager.h"

namespace MisakiDB {
class IndexBufferPoolManager final : public BufferPoolManager
{
public:
  IndexBufferPoolManager(size_t poolSize, FileStore *fileStore);

  Page *fetchIndexPage(PageIDType pageID);

  bool unpinIndexPage(PageIDType pageID, bool isDirty);

  bool flushIndexPage(PageIDType pageID);

  Page *appendNewIndexPage(PageIDType pageID);
};
}
