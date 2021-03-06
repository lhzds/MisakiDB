#pragma once
#include "globals.h"
#include "page/data_file_header.h"
#include "page/index_file_header.h"
#include "page/free_space_map_file_header.h"
#include "page/blob_file_header.h"

namespace MisakiDB {
class FileStore final {
public:
  explicit FileStore(const std::string &dbName);
  ~FileStore();
  void readRawPage(FileType fileType, PageIDType pageID, ByteType *raw);
  void writeRawPage(FileType fileType, PageIDType pageID, const ByteType *raw);
  
private:
  inline FileSizeType pageIDToOffset(PageIDType pageID);
  
  void readRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                          ByteType *raw, const std::string &fileName);
  void writeRawPage_helper(std::fstream &fileIO, PageIDType pageID,
                           const ByteType *raw, const std::string &fileName);
  
  void initFileStruct();
  void createFile(const std::string &fileName);
  
  std::fstream m_indexFileIO;
  std::fstream m_dataFileIO;
  std::fstream m_freeSpaceMapFileIO;
  std::fstream m_blobFileIO;
};
}


