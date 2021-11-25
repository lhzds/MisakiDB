#include "gtest/gtest.h"
#include "file_manager/index_file_manager.h"
#include "fileapi.h"

namespace MisakiDB {
TEST(IndexFileManagerTest, IndexFileManagerTest1) {
  std::string dbName {"test_db"};
  DeleteFileA((dbName + "\\idx.mdb").c_str());
  DeleteFileA((dbName + "\\dt.mdb").c_str());
  DeleteFileA((dbName + "\\fs_map.mdb").c_str());
  DeleteFileA((dbName + "\\blob.mdb").c_str());
  RemoveDirectoryA(dbName.c_str());
  
  int poolSize = 10;
  auto fileStore = new FileStore{dbName};
  auto ibpm = new IndexBufferPoolManager(poolSize, fileStore);
  auto ifm = new IndexFileManager{ibpm};
  Page *page = nullptr;
  
  for (int i = 1; i <= 2 * poolSize; ++i) {
    page = ifm->newIndexPage();
    ASSERT_EQ(i, page->getPageID());
    ASSERT_FALSE(reinterpret_cast<IndexFileFreePage *>(page->getData())->isFreePage());
    ASSERT_TRUE(ifm->unpinIndexPage(page->getPageID(), false));
  }
  
  for (int i = 1; i <= poolSize; ++i) {
    ASSERT_TRUE(ifm->deleteIndexPage(i * 2 - 1));
  }
  
  for (int i = poolSize; i >= 1; --i) {
    page = ifm->newIndexPage();
    ASSERT_EQ(i * 2 - 1, page->getPageID());
    ASSERT_FALSE(reinterpret_cast<IndexFileFreePage *>(page->getData())->isFreePage());
    ASSERT_TRUE(ifm->unpinIndexPage(page->getPageID(), false));
  }
  
  page = ifm->newIndexPage();
  ASSERT_EQ(2 * poolSize + 1, page->getPageID());
  ASSERT_FALSE(reinterpret_cast<IndexFileFreePage *>(page->getData())->isFreePage());
  ASSERT_TRUE(ifm->unpinIndexPage(page->getPageID(), false));
  
  delete ifm;
  delete ibpm;
  delete fileStore;
}
}