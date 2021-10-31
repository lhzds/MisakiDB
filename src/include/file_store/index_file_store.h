#pragma once
#include "globals.h"
#include "file_store.h"

namespace MisakiDB {
class IndexFileStore : public FileStore {
public:
  explicit IndexFileStore(const std::string &filename);
  ~IndexFileStore();
  void getRawPage(PageIDType pageID, ByteType *raw);
  void writeRawPage(PageIDType pageID, const ByteType *raw);

private:
  virtual void close() override;
  
  std::fstream m_indexFileIO;
  std::string m_indexFilename;
};
}
