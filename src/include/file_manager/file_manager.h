#pragma once
#include "globals.h"

namespace MisakiDB {
class FileManager {
public:
  FileManager();
  virtual RawPage getRawPage(PageIDType pageID) const = 0;
  virtual PageIDType allocatePage() = 0;
  virtual void deallocatePage(PageIDType pageID) = 0;
};
}


