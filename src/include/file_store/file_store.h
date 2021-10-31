#pragma once
#include "globals.h"

namespace MisakiDB {
class FileStore {
protected:
  virtual void close() = 0;
  
  static FileSizeType pageIDToOffset(PageIDType pageID);
};
}


