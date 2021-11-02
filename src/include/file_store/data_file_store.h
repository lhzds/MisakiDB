#pragma once
#include "globals.h"
#include "file_store.h"
#include "page/free_space_map_file_header.h"

#include <fstream>

namespace MisakiDB {
class DataFileStore final : public FileStore {
public:
  explicit DataFileStore(const std::string &dbName);
  ~DataFileStore() override;
  
  void readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) override;
  void writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) override;

protected:
  void initFileStruct(const std::string &dbName) override;

private:
  std::fstream m_freeSpaceMapFileIO;
  std::fstream m_dataFileIO;
};
}

