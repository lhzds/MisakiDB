#include "database.h"

namespace MisakiDB{
DataBase::DataBase(const std::string &databaseName, const Options &options)
    :m_databaseName { databaseName } { }

bool DataBase::get(const KeyType &key, ValueType &value) {
  std::shared_lock<std::shared_mutex> lock { this->m_readWriteLock };

  RecordIDType recordID { getRecordID_helper(key) };
  if (INVALID_RECORD_ID == recordID) return false;

  value = getValue_helper(recordID);
  return true;
}

bool DataBase::remove(const KeyType &key) {
  std::unique_lock<std::shared_mutex> lock { this->m_readWriteLock };

  RecordIDType recordID { removeRecordID_helper(key) };
  if (INVALID_RECORD_ID == recordID) return false;

  removeRecord_helper(recordID);
  return true;
}

bool DataBase::exist(const KeyType &key) {
  std::shared_lock<std::shared_mutex> lock { this->m_readWriteLock };

  return getRecordID_helper(key) not_eq INVALID_RECORD_ID;
}

void DataBase::set(const KeyType &key, const ValueType &value) {
  std::unique_lock<std::shared_mutex> lock { this->m_readWriteLock };

  remove(key);

  RecordIDType recordID { addRecord_helper(key, value) };
  addRecordID_helper(recordID);
}

std::string DataBase::getName() const { return this->m_databaseName; }

uint64_t DataBase::inuse() const { return this->m_inuse; }

void DataBase::use() { ++this->m_inuse; }

void DataBase::unuse() { --this->m_inuse; }
}
