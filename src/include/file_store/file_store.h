#pragma once
#include "globals.h"

namespace MisakiDB {
class FileStore {
public:
  virtual ~FileStore()=default;
  virtual void readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) = 0;
  virtual void writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) = 0;
  
protected:
  static FileSizeType pageIDToOffset(PageIDType pageID);
  static void readRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                 ByteType *raw, const std::string &fileName);
  static void writeRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                  const ByteType *raw, const std::string &fileName);
  
  virtual void initFileStruct(const std::string &dbName) = 0;
};
}


