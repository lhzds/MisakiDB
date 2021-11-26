#pragma once
#include "globals.h"
#include "options.h"
#include "generic_key.h"
#include "index/b_plus_tree.h"
#include "data_accessor.h"
#include "file_store/file_store.h"
#include "buffer/index_buffer_pool_manager.h"
#include "buffer/data_buffer_pool_manager.h"
#include "util.h"

namespace MisakiDB {
using BPlusTreeIndex = BPlusTree<GenericKey<RECORD_KEY_SIZE>, RecordIDType,
                                 GenericComparator<RECORD_KEY_SIZE>>;
class DataBase
{
public:
  DataBase(const std::string &databaseName, const Options &options);
  std::optional<std::string> get(std::string &key);
  bool remove(std::string &key);
  bool exist(std::string &key);
  void set(std::string &key, const std::string &value);

  void use();
  void unuse();
  uint64_t inuse() const;

  std::string getName() const;

protected:
  void processKey(std::string &key);

private:
  std::uint64_t m_inuse { 0 };
  std::string m_databaseName;

  FileStore m_fileStore;
  IndexBufferPoolManager m_indexBufferPoolManager;
  DataBufferPoolManager m_dataBufferPoolManager;
  IndexFileManager m_indexFileManager;
  DataFileManager m_dataFileManager;
  BPlusTreeIndex m_bPlusTree;
  DataAccessor m_dataAccessor;
};
}
