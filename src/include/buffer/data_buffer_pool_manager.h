#pragma once
#include "globals.h"
#include "buffer_pool_manager.h"

namespace MisakiDB {
class DataBufferPoolManager final : public BufferPoolManager
{
public:
  DataBufferPoolManager(size_t poolSize, FileStore *fileStore);

  Page *fetchDataPage(PageIDType pageID);

  bool unpinDataPage(PageIDType pageID, bool isDirty);

  bool flushDataPage(PageIDType pageID);

  Page *appendNewDataPage(PageIDType pageID);

  Page *fetchFreeSpaceMapPage(PageIDType pageID);

  bool unpinFreeSpaceMapPage(PageIDType pageID, bool isDirty);

  bool flushFreeSpaceMapPage(PageIDType pageID);

  Page *appendNewFreeSpaceMapPage(PageIDType pageID);
};
}
