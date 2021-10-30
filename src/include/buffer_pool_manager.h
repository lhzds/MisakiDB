#pragma once
#include "globals.h"
#include "page/page.h"
#include "file_manager/file_manager.h"

namespace MisakiDB {
class BufferPoolManager
{
public:
  BufferPoolManager();
  Page getPage(PageIDType pageID);
  bool removePage(PageIDType pageID);
  Page addPage(PageIDType pageID);

private:
  FileManager &fileManager;
};
}
