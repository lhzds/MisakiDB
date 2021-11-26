#pragma once
#include "globals.h"
#include "file_manager/data_file_manager.h"

namespace MisakiDB {
class DataAccessor
{
public:
  explicit DataAccessor(DataFileManager *dataFileManager);
  // TODO: 读失败，返回false
  std::optional<std::string> getRecordValue(const std::string &key, RecordIDType recordID) const;
  // TODO: key, value一起删除
  void removeRecord(RecordIDType recordID);
  // TODO: key, value
  RecordIDType insertRecord(const std::string &key, const std::string &value);
  
private:
  // for blob
  std::string insertBlob(const std::string &blob);
  std::string getBlob(const std::string &blobFirstPageID) const;
  void removeBlob(const std::string &blobFirstPageID);
  
  DataFileManager *m_dataFileManager;
};
}

