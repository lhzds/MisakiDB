#include "fileapi.h"
#include "gtest/gtest.h"
#include "buffer/buffer_pool_manager.h"

namespace MisakiDB {
// Check whether pages containing terminal characters can be recovered
TEST(BufferPoolManagerTest, BinaryDataTest) {
  const std::string dbName {"test_db"};
  DeleteFileA((dbName + "\\idx.mdb").c_str());
  DeleteFileA((dbName + "\\dt.mdb").c_str());
  DeleteFileA((dbName + "\\fs_map.mdb").c_str());
  RemoveDirectoryA(dbName.c_str());
  
  const size_t poolSize {10};
  std::random_device r;
  std::default_random_engine rng(r());
  std::uniform_int_distribution<char> uniformDist(0);
  
  auto *fileStore = new FileStore(dbName);
  auto *bpm = new BufferPoolManager(poolSize, fileStore);
  
  // Scenario: Fetch the header page.
  auto * headerPage = bpm->fetchPage(FILE_TYPE::INDEX, 0);
  ASSERT_NE(nullptr, headerPage);
  EXPECT_EQ(FILE_TYPE::INDEX, headerPage->getFileType());
  EXPECT_EQ(0, headerPage->getPageID());
  auto *header = reinterpret_cast<IndexFileHeader *>(headerPage->getData());
  EXPECT_EQ(1, header->getNextPageID());
  
  // Scenario: We can create a new page.
  auto *page1 = bpm->appendNewPage(FILE_TYPE::INDEX, header->getNextPageID());
  header->incNextPageID();
  ASSERT_NE(nullptr, page1);
  EXPECT_EQ(FILE_TYPE::INDEX, page1->getFileType());
  EXPECT_EQ(1, page1->getPageID());
  
  char randomBinaryData[PAGE_SIZE];
  // Generate random binary data
  for (char &i : randomBinaryData) {
    i = uniformDist(rng);
  }
  
  // Insert terminal characters both in the middle and at end
  randomBinaryData[PAGE_SIZE / 2] = '\0';
  randomBinaryData[PAGE_SIZE - 1] = '\0';
  
  // Scenario: Once we have a page, we should be able to read and write content.
  std::memcpy(page1->getData(), randomBinaryData, PAGE_SIZE);
  EXPECT_EQ(0, std::memcmp(page1->getData(), randomBinaryData, PAGE_SIZE));

  // Scenario: We should be able to create new pages until we fill up the buffer pool.
  while (header->getNextPageID() < poolSize) {
    EXPECT_NE(nullptr, bpm->appendNewPage(FILE_TYPE::INDEX, header->getNextPageID()));
    header->incNextPageID();
  }
  
  // Scenario: Once the buffer pool is full, we should not be able to create any new pages.
  EXPECT_EQ(nullptr, bpm->appendNewPage(FILE_TYPE::INDEX, header->getNextPageID()));
  
  // Scenario:
  // Upin page 1, then pin a new page to evist page1 to disk.
  EXPECT_EQ(true, bpm->unpinPage(FILE_TYPE::INDEX, 1, true));
  EXPECT_NE(nullptr, bpm->appendNewPage(FILE_TYPE::INDEX, header->getNextPageID()));
  header->incNextPageID();
  
  // If buffer pool is full and all page pinned, now fetching page 1 from the disk will fail.
  EXPECT_EQ(nullptr, bpm->fetchPage(FILE_TYPE::INDEX, 1));
  
  // Unpin page 0, then we can fetch page 1 into pool and  write back page 0 to disk.
  EXPECT_EQ(true, bpm->unpinPage(FILE_TYPE::INDEX, 0, true));
  // Check the page 1 is same as before it was written to disk or not
  page1 = bpm->fetchPage(FILE_TYPE::INDEX, 1);
  EXPECT_EQ(0, memcmp(page1->getData(), randomBinaryData, PAGE_SIZE));
  
  // Unpin page 1, then we can fetch page 0 into pool and  write back page 1 to disk.
  EXPECT_EQ(true, bpm->unpinPage(FILE_TYPE::INDEX, 1, false));
  // Check the page 0 is same as before it was written to disk or not
  header = reinterpret_cast<IndexFileHeader *>(bpm->fetchPage(FILE_TYPE::INDEX, 0)->getData());
  EXPECT_EQ(header->getNextPageID(), 11);
  
  delete bpm;
  delete fileStore;
}
}
