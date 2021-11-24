#pragma once
#include "globals.h"
#include "file_manager/data_file_manager.h"

namespace MisakiDB {
class DataAccessor
{
public:
  explicit DataAccessor(DataFileManager *dataFileManager);
  std::string getData(RecordIDType recordID) const;
  void removeData(RecordIDType recordID);
  RecordIDType insertData(const std::string &value);
  
private:
  DataFileManager *m_dataFileManager;
};
}

