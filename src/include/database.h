#pragma once
#include "globals.h"
#include "options.h"
#include "index/b_plus_tree.h"
#include "data_accessor.h"
#include "file_store/file_store.h"
#include "buffer/index_buffer_pool_manager.h"
#include "buffer/data_buffer_pool_manager.h"

namespace MisakiDB {
class DataBase
{
public:
  DataBase(const std::string &databaseName, const Options &options);
  bool get(const KeyType &key, std::string &value);
  bool remove(const KeyType &key);
  bool exist(const KeyType &key);
  void set(const KeyType &key, const std::string &value);

  void use();
  void unuse();
  uint64_t inuse() const;

  std::string getName() const;

protected:
  RecordIDType getRecordID(const KeyType &key) const;
  std::string getValue(RecordIDType recordID) const;

  RecordIDType removeRecordID(const KeyType &key);
  void removeRecord(RecordIDType recordID);

  RecordIDType addRecord(const KeyType &key, const std::string &value);
  void addRecordID(RecordIDType recordID);

private:
  std::uint64_t m_inuse { 0 };
  std::string m_databaseName;

  FileStore m_fileStore;
  IndexBufferPoolManager m_indexBufferPoolManager;
  DataBufferPoolManager m_dataBufferPoolManager;
  BPlusTree m_bPlusTree;
  DataAccessor m_dataAccessor;
};
}
