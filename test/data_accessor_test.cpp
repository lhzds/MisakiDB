#include "gtest/gtest.h"
#include "fileapi.h"
#include "data_accessor.h"
#include "page/data_file_page.h"
#include "file_manager/data_file_manager.h"

namespace MisakiDB {
class DataAccessorTest : public testing::Test {
protected:
  void SetUp() override {
    DeleteFileA((dbName + "\\idx.mdb").c_str());
    DeleteFileA((dbName + "\\dt.mdb").c_str());
    DeleteFileA((dbName + "\\fs_map.mdb").c_str());
    RemoveDirectoryA(dbName.c_str());
    
    fileStore = new FileStore(dbName);
  }
  
  void TearDown() override {
    delete fileStore;
  }
  
  FileStore *fileStore;
  const std::string dbName{"test_db"};
};

TEST_F(DataAccessorTest, DataAccessorTest1) {
  DataBufferPoolManager *dbpm { new DataBufferPoolManager(30, fileStore) };
  DataFileManager *dfm { new DataFileManager(dbpm) };
  DataAccessor *da { new DataAccessor(dfm) };
  
  int recordSize = 100 - DataFilePage::SLOT_SIZE;
  int recordNumPerPage = (PAGE_SIZE - DataFilePage::MIN_FIXED_SIZE) / 100;
  // insert
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      std::string record = std::to_string(j);
      record += std::string(recordSize - record.length(), record.at(0));
      RecordIDType recordID = da->insertData(record);
      ASSERT_EQ(i, recordID.pageID);
      ASSERT_EQ(j, recordID.slotArrayIndex);
    }
  }
  // get
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      std::string expected = std::to_string(j);
      expected += std::string(recordSize - expected.length(), expected.at(0));
      RecordIDType recordID;
      recordID.pageID = i;
      recordID.slotArrayIndex = j;
      ASSERT_EQ(da->getData(recordID), expected);
    }
  }
  int sieve = 2;
  // delete
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      if (j % sieve == 0) {
        continue;
      }
      RecordIDType recordID;
      recordID.pageID = i;
      recordID.slotArrayIndex = j;
      da->removeData(recordID);
    }
  }
  // reinsert
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      if (j % sieve == 0) {
        continue;
      }
      std::string record = std::to_string(j);
      record += std::string(recordSize - record.length(), record.at(0));
      RecordIDType recordID = da->insertData(record);
      ASSERT_EQ(i, recordID.pageID);
      ASSERT_EQ(j, recordID.slotArrayIndex);
    }
  }
  
  delete dfm;
  delete dbpm;
}
}
