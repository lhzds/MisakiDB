#pragma once
#include "globals.h"
#include "page/data_file_page_header.h"
#include "page/index_file_page_header.h"
#include "page/free_space_map_file_header.h"

namespace MisakiDB {
class FileStore final {
public:
  FileStore(const std::string &dbName);
  ~FileStore();
  void readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw);
  void writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw);
  
private:
  FileSizeType pageIDToOffset(PageIDType pageID);
  
  void readRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                 ByteType *raw, const std::string &fileName);
  void writeRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                  const ByteType *raw, const std::string &fileName);
  
  void initFileStruct();
  void createFile(const std::string &fileName);
  
  std::fstream m_indexFileIO;
  std::fstream m_dataFileIO;
  std::fstream m_freeSpaceMapFileIO;
};
}


