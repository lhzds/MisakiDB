#include "gtest/gtest.h"
#include "page/free_space_map_file_page.h"
#include "page/data_file_page.h"
#include "file_manager/data_file_manager.h"
#include "fileapi.h"

namespace MisakiDB {
TEST(DataFileManagerTest, DataFileManagerTest1) {
  std::string dbName {"test_db"};
  DeleteFileA((dbName + "\\idx.mdb").c_str());
  DeleteFileA((dbName + "\\dt.mdb").c_str());
  DeleteFileA((dbName + "\\fs_map.mdb").c_str());
  DeleteFileA((dbName + "\\blob.mdb").c_str());
  RemoveDirectoryA(dbName.c_str());
  
  int poolSize = 10;
  auto fileStore = new FileStore{dbName};
  auto dbpm = new DataBufferPoolManager(poolSize, fileStore);
  auto dfm = new DataFileManager{dbpm};
  
  Page *page = nullptr;
  // allocate data pages and fill up all their free space
  for (int i = 1; i <= 2 * FreeSpaceMapFilePage::MAX_SIZE; ++i) {
    page = dfm->allocateDataPage(DataFilePage::MAX_RECORD_SIZE);
    ASSERT_EQ(i, page->getPageID());
    ASSERT_TRUE(dfm->unpinDataPage(page->getPageID(), false));
  }
  // make sure all previous allocated page full
  page = dfm->allocateDataPage(1);
  ASSERT_EQ(2 * FreeSpaceMapFilePage::MAX_SIZE + 1, page->getPageID());
  ASSERT_TRUE(dfm->unpinDataPage(page->getPageID(), false));
  page = dfm->allocateDataPage(DataFilePage::MAX_RECORD_SIZE - 1 - DataFilePage::SLOT_SIZE);
  ASSERT_EQ(2 * FreeSpaceMapFilePage::MAX_SIZE + 1, page->getPageID());
  ASSERT_TRUE(dfm->unpinDataPage(page->getPageID(), false));
  // decrease free space of all pages whose page id are odd
  for (int i = 1; i <= FreeSpaceMapFilePage::MAX_SIZE; ++i) {
    dfm->addFreeSpace(i * 2 - 1, i);
  }
  
  for (int i = 1; i <= FreeSpaceMapFilePage::MAX_SIZE / 2; ++i) {
    page = dfm->allocateDataPage(i);
    ASSERT_EQ(i * 2 - 1, page->getPageID());
    ASSERT_TRUE(dfm->unpinDataPage(page->getPageID(), false));
  
    page = dfm->allocateDataPage(FreeSpaceMapFilePage::MAX_SIZE + 1 - i);
    ASSERT_EQ(FreeSpaceMapFilePage::MAX_SIZE * 2 - (i * 2 - 1), page->getPageID());
    ASSERT_TRUE(dfm->unpinDataPage(page->getPageID(), false));
  }
  
  delete dfm;
  delete dbpm;
  delete fileStore;
}
}