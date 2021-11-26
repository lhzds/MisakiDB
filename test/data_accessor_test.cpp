#include "gtest/gtest.h"
#include "fileapi.h"
#include "data_accessor.h"
#include "page/data_file_page.h"

namespace MisakiDB {
class DataAccessorTest : public testing::Test {
protected:
  void SetUp() override {
    DeleteFileA((dbName + "\\idx.mdb").c_str());
    DeleteFileA((dbName + "\\dt.mdb").c_str());
    DeleteFileA((dbName + "\\fs_map.mdb").c_str());
    DeleteFileA((dbName + "\\blob.mdb").c_str());
    RemoveDirectoryA(dbName.c_str());
    
    fileStore = new FileStore(dbName);
  }
  
  void TearDown() override {
    delete fileStore;
  }
  
  FileStore *fileStore;
  const std::string dbName{"test_db"};
};

TEST_F(DataAccessorTest, slobTest) {
  DataBufferPoolManager *dbpm { new DataBufferPoolManager(30, fileStore) };
  DataFileManager *dfm { new DataFileManager(dbpm) };
  DataAccessor *da { new DataAccessor(dfm) };
  
  const int recordValueSize = 100 - DataFilePage::SLOT_SIZE - RECORD_KEY_SIZE;
  const int recordNumPerPage = (PAGE_SIZE - DataFilePage::MIN_FIXED_SIZE) / 100;
  // insert
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      std::string key = std::to_string(i * 10000 + j);
      key += std::string(RECORD_KEY_SIZE - key.length(), key.at(0));
      
      std::string value = std::to_string(j);
      value += std::string(recordValueSize - value.length(), value.at(0));
      
      RecordIDType recordID = da->insertRecord(key, value);
      ASSERT_EQ(i, recordID.pageID);
      ASSERT_EQ(j, recordID.slotArrayIndex);
    }
  }
  // get
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      std::string key = std::to_string(i * 10000 + j);
      key += std::string(RECORD_KEY_SIZE - key.length(), key.at(0));
  
      std::string value = std::to_string(j);
      value += std::string(recordValueSize - value.length(), value.at(0));
      
      RecordIDType recordID;
      recordID.pageID = i;
      recordID.slotArrayIndex = j;
      ASSERT_EQ(da->getRecordValue(key, recordID), value);
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
      da->removeRecord(recordID);
    }
  }
  
  // reinsert
  for (int i = 1; i <= 10; ++i) {
    for (int j = 0; j < recordNumPerPage; ++j) {
      if (j % sieve == 0) {
        continue;
      }
      std::string key = std::to_string(i * 10000 + j);
      key += std::string(RECORD_KEY_SIZE - key.length(), key.at(0));
  
      std::string value = std::to_string(j);
      value += std::string(recordValueSize - value.length(), value.at(0));
      
      RecordIDType recordID = da->insertRecord(key, value);
      ASSERT_EQ(i, recordID.pageID);
      ASSERT_EQ(j, recordID.slotArrayIndex);
    }
  }
  
  delete da;
  delete dfm;
  delete dbpm;
}

TEST_F(DataAccessorTest, blobTest) {
  DataBufferPoolManager *dbpm { new DataBufferPoolManager(30, fileStore) };
  DataFileManager *dfm { new DataFileManager(dbpm) };
  DataAccessor *da { new DataAccessor(dfm) };

  size_t blobMinSize = DataFilePage::MAX_RECORD_SIZE + 1;

  std::vector<RecordIDType> inserteds;
  for (int i = 0; i < 10; ++i) {
    std::string key = std::to_string(i);
    key += std::string(RECORD_KEY_SIZE - key.length(), key.at(0));
    std::string value;
    for (int j = 0; j <= i; ++j) {
      value.append(std::string(blobMinSize / 2, 'a' + j));
      value.append(std::string(blobMinSize - blobMinSize / 2, 'a' + j - 1));
    }
    inserteds.emplace_back(da->insertRecord(key, value));
  }

  int sieve = 2;
  for (int i = 0; i < 10; ++i) {
    if (i % sieve == 0) {
      da->removeRecord(inserteds[i]);
    }
  }

  for (int i = 0; i < 10; ++i) {
    if (i % sieve != 0) {
      std::string key = std::to_string(i);
      key += std::string(RECORD_KEY_SIZE - key.length(), key.at(0));
      std::string value;
      for (int j = 0; j <= i; ++j) {
        value.append(std::string(blobMinSize / 2, 'a' + j));
        value.append(std::string(blobMinSize - blobMinSize / 2, 'a' + j - 1));
      }
      auto result = da->getRecordValue(key, inserteds[i]);
      ASSERT_TRUE(result.has_value());
      ASSERT_EQ(value, *result);
    }
  }

  delete da;
  delete dfm;
  delete dbpm;
}
}
