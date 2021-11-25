#include "database.h"

namespace MisakiDB{
DataBase::DataBase(const std::string &databaseName, const Options &options)
    :m_databaseName { databaseName },
      m_fileStore { databaseName },
      m_indexBufferPoolManager { options.getIndexBufferPoolSize(), &m_fileStore },
      m_dataBufferPoolManager { options.getDataBufferPoolSize(), &m_fileStore },
      m_bPlusTree { &m_indexBufferPoolManager } { }

bool DataBase::get(const KeyType &key, std::string &value) {
  RecordIDType recordID { getRecordID(key) };
  if (INVALID_RECORD_ID.recordID == recordID.recordID) return false;

  value = getValue(recordID);
  return true;
}

bool DataBase::remove(const KeyType &key) {
  RecordIDType recordID { removeRecordID(key) };
  if (INVALID_RECORD_ID.recordID == recordID.recordID) return false;

  removeRecord(recordID);
  return true;
}

bool DataBase::exist(const KeyType &key) {
  return getRecordID(key).recordID not_eq INVALID_RECORD_ID.recordID;
}

void DataBase::set(const KeyType &key, const std::string &value) {
  remove(key);

  RecordIDType recordID { addRecord(key, value) };
  addRecordID(recordID);
}

void DataBase::use() { ++this->m_inuse; }

void DataBase::unuse() { --this->m_inuse; }

uint64_t DataBase::inuse() const { return this->m_inuse; }

std::string DataBase::getName() const { return this->m_databaseName; }

RecordIDType DataBase::getRecordID(const KeyType &key) const {
}

std::string DataBase::getValue(RecordIDType recordID) const {
}

RecordIDType DataBase::removeRecordID(const KeyType &key) {

}

void DataBase::removeRecord(RecordIDType recordID) {

}

RecordIDType DataBase::addRecord(const KeyType &key, const std::string &value) {

}

void DataBase::addRecordID(RecordIDType recordID) {

}
}
