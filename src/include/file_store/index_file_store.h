#pragma once
#include "globals.h"
#include "file_store.h"
#include "page/index_file_page_header.h"

#include <fstream>

namespace MisakiDB {
class IndexFileStore final : public FileStore {
public:
  explicit IndexFileStore(const std::string &dbName);
  ~IndexFileStore() override;
  
  void readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) override;
  void writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) override;

protected:
  void initFileStruct(const std::string &dbName) override;
  
private:
  std::fstream m_indexFileIO;
};
}
