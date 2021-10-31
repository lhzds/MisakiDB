#pragma once
#include "globals.h"

namespace MisakiDB {
class FileStore {
protected:
  virtual void close();
  
  static FileSizeType pageIDToOffset(PageIDType pageID);
  static FileSizeType getFileSize(const std::string &filename);
  static bool isValidPageID(PageIDType pageID, const std::string &filename);
};
}


