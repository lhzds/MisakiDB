#include "database.h"

namespace MisakiDB{
DataBase::DataBase(const std::string &databaseName, const Options &options)
    :m_databaseName { databaseName },
      m_fileStore { databaseName },
      m_indexBufferPoolManager { options.getIndexBufferPoolSize(), &m_fileStore },
      m_dataBufferPoolManager { options.getDataBufferPoolSize(), &m_fileStore },
      m_bPlusTree { &m_indexBufferPoolManager } { }

bool DataBase::get(const KeyType &key, ValueType &value) {
  RecordIDType recordID { getRecordID_helper(key) };
  if (INVALID_RECORD_ID == recordID) return false;

  value = getValue_helper(recordID);
  return true;
}

bool DataBase::remove(const KeyType &key) {
  RecordIDType recordID { removeRecordID_helper(key) };
  if (INVALID_RECORD_ID == recordID) return false;

  removeRecord_helper(recordID);
  return true;
}

bool DataBase::exist(const KeyType &key) {
  return getRecordID_helper(key) not_eq INVALID_RECORD_ID;
}

void DataBase::set(const KeyType &key, const ValueType &value) {
  remove(key);

  RecordIDType recordID { addRecord_helper(key, value) };
  addRecordID_helper(recordID);
}

void DataBase::use() { ++this->m_inuse; }

void DataBase::unuse() { --this->m_inuse; }

uint64_t DataBase::inuse() const { return this->m_inuse; }

std::string DataBase::getName() const { return this->m_databaseName; }

RecordIDType DataBase::getRecordID_helper(const KeyType &key) const {
}

ValueType DataBase::getValue_helper(RecordIDType recordID) const {
}

RecordIDType DataBase::removeRecordID_helper(const KeyType &key) {

}

void DataBase::removeRecord_helper(RecordIDType recordID) {

}

RecordIDType DataBase::addRecord_helper(const KeyType &key, const ValueType &value) {

}

void DataBase::addRecordID_helper(RecordIDType recordID) {

}
}
