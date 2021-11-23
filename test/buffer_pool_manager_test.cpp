#include "fileapi.h"
#include "gtest/gtest.h"
#include "buffer/buffer_pool_manager.h"

namespace MisakiDB {
class BufferPoolManagerTest : public testing::Test {
protected:
  void SetUp() override {
    DeleteFileA((dbName + "\\idx.mdb").c_str());
    DeleteFileA((dbName + "\\dt.mdb").c_str());
    DeleteFileA((dbName + "\\fs_map.mdb").c_str());
    RemoveDirectoryA(dbName.c_str());
  
    fileStore = new FileStore(dbName);
  }
  
  virtual void TearDown() {
    delete fileStore;
  }
  
  void BinaryDataTester(FileType fileType) {
    const size_t poolSize {10};
    std::random_device r;
    std::default_random_engine rng(r());
    std::uniform_int_distribution<char> uniformDist(0);
    
    auto *bpm = new BufferPoolManager(poolSize, fileStore);
  
    // Scenario: Fetch the header page.
    auto * headerPage = bpm->fetchPage(fileType, 0);
    ASSERT_NE(nullptr, headerPage);
    EXPECT_EQ(fileType, headerPage->getFileType());
    EXPECT_EQ(0, headerPage->getPageID());
    auto *header = reinterpret_cast<IndexFileHeader *>(headerPage->getData());
    EXPECT_EQ(1, header->getNextPageID());
  
    // Scenario: We can create a new page.
    auto *page1 = bpm->appendNewPage(fileType, header->getNextPageID());
    header->incNextPageID();
    ASSERT_NE(nullptr, page1);
    EXPECT_EQ(fileType, page1->getFileType());
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
      EXPECT_NE(nullptr, bpm->appendNewPage(fileType, header->getNextPageID()));
      header->incNextPageID();
    }
  
    // Scenario: Once the buffer pool is full, we should not be able to create any new pages.
    EXPECT_EQ(nullptr, bpm->appendNewPage(fileType, header->getNextPageID()));
  
    // Scenario:
    // Unpin page 1, then pin a new page to evict page1 to disk.
    EXPECT_EQ(true, bpm->unpinPage(fileType, 1, true));
    EXPECT_NE(nullptr, bpm->appendNewPage(fileType, header->getNextPageID()));
    header->incNextPageID();
  
    // If buffer pool is full and all page pinned, now fetching page 1 from the disk will fail.
    EXPECT_EQ(nullptr, bpm->fetchPage(fileType, 1));
  
    // Unpin page 0, then we can fetch page 1 into pool and  write back page 0 to disk.
    EXPECT_EQ(true, bpm->unpinPage(fileType, 0, true));
  
    // Check the page 1 is same as before it was written to disk or not
    page1 = bpm->fetchPage(fileType, 1);
    EXPECT_EQ(0, memcmp(page1->getData(), randomBinaryData, PAGE_SIZE));
  
    // Unpin page 1, then we can fetch page 0 into pool and  write back page 1 to disk.
    EXPECT_EQ(true, bpm->unpinPage(fileType, 1, false));
  
    // Check the page 0 is same as before it was written to disk or not
    header = reinterpret_cast<IndexFileHeader *>(bpm->fetchPage(fileType, 0)->getData());
    EXPECT_EQ(header->getNextPageID(), 11);
  
    delete bpm;
  }
  
  FileStore *fileStore;
  const std::string dbName {"test_db"};
};

// Check whether pages containing terminal characters can be recovered
TEST_F(BufferPoolManagerTest, BinaryDataTest) {
  BinaryDataTester(FileType::INDEX);
  BinaryDataTester(FileType::DATA);
  BinaryDataTester(FileType::FREE_SPACE_MAP);
}

TEST_F(BufferPoolManagerTest, MultiFilesTest) {
  size_t poolSize = 50;
  auto *bpm = new BufferPoolManager(poolSize, fileStore);
  
  Page *page;
  for (int i = 1; i < 10000; ++i) {
    page = bpm->appendNewPage(FileType::INDEX, i);
    EXPECT_NE(nullptr, page);
    memcpy(page->getData(), std::to_string(i).c_str(), std::to_string(i).length() + 1);
    EXPECT_EQ(true, bpm->unpinPage(FileType::INDEX, i, true));
  
    page = bpm->appendNewPage(FileType::DATA, i);
    EXPECT_NE(nullptr, page);
    memcpy(page->getData(), std::to_string(i).c_str(), std::to_string(i).length() + 1);
    EXPECT_EQ(true, bpm->unpinPage(FileType::DATA, i, true));
  
    page = bpm->appendNewPage(FileType::FREE_SPACE_MAP, i);
    EXPECT_NE(nullptr, page);
    memcpy(page->getData(), std::to_string(i).c_str(), std::to_string(i).length() + 1);
    EXPECT_EQ(true, bpm->unpinPage(FileType::FREE_SPACE_MAP, i, true));
  }
  
  for (int i = 1; i < 10000; ++i) {
    page = bpm->fetchPage(FileType::INDEX, i);
    EXPECT_NE(nullptr, page);
    EXPECT_EQ(0, strcmp(std::to_string(i).c_str(), page->getData()));
    EXPECT_EQ(true, bpm->unpinPage(FileType::INDEX, i, false));
  
    page = bpm->fetchPage(FileType::DATA, i);
    EXPECT_NE(nullptr, page);
    EXPECT_EQ(0, strcmp(std::to_string(i).c_str(), page->getData()));
    EXPECT_EQ(true, bpm->unpinPage(FileType::DATA, i, false));
  
    page = bpm->fetchPage(FileType::FREE_SPACE_MAP, i);
    EXPECT_NE(nullptr, page);
    EXPECT_EQ(0, strcmp(std::to_string(i).c_str(), page->getData()));
    EXPECT_EQ(true, bpm->unpinPage(FileType::FREE_SPACE_MAP, i, false));
  }

  delete bpm;
}

}
