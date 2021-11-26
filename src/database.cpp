#include "database.h"

namespace MisakiDB{
DataBase::DataBase(const std::string &databaseName, const Options &options)
    :m_databaseName { databaseName },
      m_fileStore { databaseName },
      m_indexBufferPoolManager { options.getIndexBufferPoolSize(), &m_fileStore },
      m_dataBufferPoolManager { options.getDataBufferPoolSize(), &m_fileStore },
      m_indexFileManager { &m_indexBufferPoolManager },
      m_dataFileManager { &m_dataBufferPoolManager },
      m_bPlusTree { &m_indexFileManager, GenericComparator<RECORD_KEY_SIZE> { } },
      m_dataAccessor { &m_dataFileManager } { }

bool DataBase::get(std::string &key, std::string &value) {
  processKey(key);

  RecordIDType recordID { getRecordID(key) };
  if (INVALID_RECORD_ID.recordID == recordID.recordID) return false;

  value = getValue(recordID);
  return true;
}

bool DataBase::remove(std::string &key) {
  processKey(key);

  RecordIDType recordID { removeRecordID(key) };
  if (INVALID_RECORD_ID.recordID == recordID.recordID) return false;

  removeRecord(recordID);
  return true;
}

bool DataBase::exist(std::string &key) {
  processKey(key);

  return getRecordID(key).recordID not_eq INVALID_RECORD_ID.recordID;
}

void DataBase::set(std::string &key, const std::string &value) {
  processKey(key);

  remove(key);

  RecordIDType recordID { addRecord(key, value) };
  addRecordID(recordID);
}

void DataBase::use() { ++this->m_inuse; }

void DataBase::unuse() { --this->m_inuse; }

uint64_t DataBase::inuse() const { return this->m_inuse; }

std::string DataBase::getName() const { return this->m_databaseName; }

void DataBase::processKey(std::string &key) {
  if (key.length() > RECORD_KEY_SIZE) key = key.substr(0, RECORD_KEY_SIZE);
  else key = key + std::string(RECORD_KEY_SIZE - key.length(), '\0');
}

RecordIDType DataBase::getRecordID(const std::string &key) const {

}

std::string DataBase::getValue(RecordIDType recordID) const {

}

RecordIDType DataBase::removeRecordID(const std::string &key) {

}

void DataBase::removeRecord(RecordIDType recordID) {

}

RecordIDType DataBase::addRecord(const std::string &key, const std::string &value) {

}

void DataBase::addRecordID(RecordIDType recordID) {

}
}
