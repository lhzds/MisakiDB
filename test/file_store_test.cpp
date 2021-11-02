#include "gtest/gtest.h"
#include "file_store/data_file_store.h"
#include "file_store/index_file_store.h"
namespace MisakiDB {
TEST(FileStoreTest, IndexFileStoreTest) {
  auto indexFileStore = new IndexFileStore{"test_db"};
//  char rawHeader[PAGE_SIZE];
//  indexFileStore->readRawPage(FILE_TYPE::INDEX, 0, rawHeader);
//  IndexFileHeader *indexFileHeader = reinterpret_cast<IndexFileHeader *>(rawHeader);
//  EXPECT_EQ(indexFileHeader->getNextPageId(), INVALID_PAGE_ID);
//  EXPECT_EQ(indexFileHeader->getRootPageId(), INVALID_PAGE_ID);
//  EXPECT_EQ(indexFileHeader->getFreePageListHeader(), INVALID_PAGE_ID);
//
//  char rawPage1[PAGE_SIZE];
//  memset(rawPage1, 0x3f, PAGE_SIZE);
//  indexFileStore->writeRawPage(FILE_TYPE::INDEX, 1, rawPage1);
//  char rawPage2[PAGE_SIZE];
//  indexFileStore->readRawPage(FILE_TYPE::INDEX, 1, rawPage2);
//  EXPECT_EQ(strcmp(rawPage1, rawPage2), 0);
//
//  // close file and then reopen
//  delete indexFileStore;
//  indexFileStore = new IndexFileStore{"test_db"};
//  indexFileStore->readRawPage(FILE_TYPE::INDEX, 0, rawHeader);
//  indexFileHeader = reinterpret_cast<IndexFileHeader *>(rawHeader);
//  EXPECT_EQ(indexFileHeader->getNextPageId(), INVALID_PAGE_ID);
//  EXPECT_EQ(indexFileHeader->getRootPageId(), INVALID_PAGE_ID);
//  EXPECT_EQ(indexFileHeader->getFreePageListHeader(), INVALID_PAGE_ID);
//
//  indexFileStore->readRawPage(FILE_TYPE::INDEX, 1, rawPage2);
//  EXPECT_EQ(strcmp(rawPage1, rawPage2), 0);
}

TEST(FileStoreTest, DataFileStoreTest) {

}
}

