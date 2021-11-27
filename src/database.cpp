#include "database.h"

namespace MisakiDB{
DataBase::DataBase(const std::string &databaseName)
    :m_databaseName { databaseName }, m_fileStore { databaseName },
      m_indexBufferPoolManager { INDEX_BUFFER_POOL_SIZE, &m_fileStore },
      m_dataBufferPoolManager { DATA_BUFFER_POOL_SIZE, &m_fileStore },
      m_indexFileManager { &m_indexBufferPoolManager },
      m_dataFileManager { &m_dataBufferPoolManager },
      m_bPlusTree { &m_indexFileManager, GenericComparator<RECORD_KEY_SIZE> { } },
      m_dataAccessor { &m_dataFileManager } { }

std::optional<std::string> DataBase::get(std::string &key) {
  processKey(key);

  // Get record id from b+ tree
  auto result { this->m_bPlusTree.getValue(key) };
  RecordIDType recordID;
  if (result.has_value()) recordID = result.value();
  else return std::nullopt;

  return m_dataAccessor.getRecordValue(key, recordID);
}

bool DataBase::remove(std::string &key) {
  processKey(key);

  // Get record id from b+ tree
  auto result { this->m_bPlusTree.remove(key) };
  if (not result.has_value()) return false;

  // Remove it from the data file
  this->m_dataAccessor.removeRecord(result.value());

  return true;
}

bool DataBase::exist(std::string &key) {
  processKey(key);

  return this->m_bPlusTree.getValue(key).has_value();
}

void DataBase::set(std::string &key, const std::string &value) {
  processKey(key);

  // Set = Remove + Insert
  remove(key);

  RecordIDType recordID { this->m_dataAccessor.insertRecord(key, value) };
  if (not this->m_bPlusTree.insert(key, recordID)) {
    this->m_dataAccessor.removeRecord(recordID);
  }
}

void DataBase::use() { ++this->m_inuse; }

void DataBase::unuse() { --this->m_inuse; }

uint64_t DataBase::inuse() const { return this->m_inuse; }

std::string DataBase::getName() const { return this->m_databaseName; }

void DataBase::processKey(std::string &key) {
  trim(key);
  if (key.length() > RECORD_KEY_SIZE) key = key.substr(0, RECORD_KEY_SIZE);
  else key = key + std::string(RECORD_KEY_SIZE - key.length(), '\0');
}
}
