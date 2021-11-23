#include "gtest/gtest.h"
#include "file_store/file_store.h"
#include "fileapi.h"

namespace MisakiDB {
TEST(FileStoreTest, FileStoreTest1) {
  std::string dbName {"test_db"};
  DeleteFileA((dbName + "\\idx.mdb").c_str());
  DeleteFileA((dbName + "\\dt.mdb").c_str());
  DeleteFileA((dbName + "\\fs_map.mdb").c_str());
  RemoveDirectoryA(dbName.c_str());
  
  auto fileStore = new FileStore{dbName};

  // test init headers
  char rawHeader[PAGE_SIZE];
  fileStore->readRawPage(FileType::INDEX, 0, rawHeader);
  auto *indexFileHeader = reinterpret_cast<IndexFileHeader *>(rawHeader);
  EXPECT_EQ(indexFileHeader->getNextPageID(), 1);
  EXPECT_EQ(indexFileHeader->getRootPageID(), INVALID_PAGE_ID);
  EXPECT_EQ(indexFileHeader->getFreePageListHeader(), INVALID_PAGE_ID);

  fileStore->readRawPage(FileType::FREE_SPACE_MAP, 0, rawHeader);
  auto *fsMapFileHeader = reinterpret_cast<FreeSpaceMapFileHeader *>(rawHeader);
  EXPECT_EQ(fsMapFileHeader->getNextPageID(), 1);
  
  fileStore->readRawPage(FileType::DATA, 0, rawHeader);
  auto *dataFileHeader = reinterpret_cast<DataFileHeader *>(rawHeader);
  EXPECT_EQ(dataFileHeader->getNextPageID(), 1);

  // test read and write
  char rawPage1[PAGE_SIZE];
  memset(rawPage1, 0x4f, PAGE_SIZE);
  rawPage1[PAGE_SIZE - 1] = '\0';
  fileStore->writeRawPage(FileType::INDEX, 1, rawPage1);
  char rawPage2[PAGE_SIZE];
  fileStore->readRawPage(FileType::INDEX, 1, rawPage2);
  EXPECT_EQ(strcmp(rawPage1, rawPage2), 0);

  rawPage1[5] = 'a';
  fileStore->writeRawPage(FileType::INDEX, 1, rawPage1);
  fileStore->readRawPage(FileType::INDEX, 1, rawPage2);
  EXPECT_EQ(strcmp(rawPage1, rawPage2), 0);

  // close file and then reopen
  delete fileStore;
  fileStore = new FileStore{"test_db"};
  fileStore->readRawPage(FileType::INDEX, 0, rawHeader);
  indexFileHeader = reinterpret_cast<IndexFileHeader *>(rawHeader);
  EXPECT_EQ(indexFileHeader->getNextPageID(), 1);
  EXPECT_EQ(indexFileHeader->getRootPageID(), INVALID_PAGE_ID);
  EXPECT_EQ(indexFileHeader->getFreePageListHeader(), INVALID_PAGE_ID);

  fileStore->readRawPage(FileType::INDEX, 1, rawPage2);
  EXPECT_EQ(strcmp(rawPage1, rawPage2), 0);
}
}

