#pragma once
#include "globals.h"
#include "file_manager/data_file_manager.h"

namespace MisakiDB {
class DataAccessor
{
public:
  explicit DataAccessor(DataFileManager *dataFileManager);
  std::optional<std::string> getRecordValue(const std::string &key, RecordIDType recordID) const;
  void removeRecord(RecordIDType recordID);
  RecordIDType insertRecord(const std::string &key, const std::string &value);
  
private:
  // for blob
  std::string insertBlob(const std::string &key, const std::string &blob);
  std::optional<std::string> getBlob(const std::string &key, const std::string &blobFirstPageID) const;
  void removeBlob(const std::string &blobFirstPageID);
  
  std::atomic_int64_t m_nextBlobID = 0;
  DataFileManager *m_dataFileManager;
};
}

