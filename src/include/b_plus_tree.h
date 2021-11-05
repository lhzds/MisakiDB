#pragma once
#include "globals.h"
#include "buffer/index_buffer_pool_manager.h"

namespace MisakiDB {
class BPlusTree {
public:
  BPlusTree(IndexBufferPoolManager *buffer_pool_manager);

  void insertEntry(const KeyType &key, RecordIDType rid);

  void deleteEntry(const KeyType &key, RecordIDType rid);

  void scanKey(const KeyType &key, std::vector<RecordIDType> *result);
};
}
