#include "gtest/gtest.h"
#include "index/b_plus_tree.h"
#include "fileapi.h"

namespace MisakiDB {
class BPlusTreeTest : public testing::Test {
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
  const std::string dbName {"test_db"};
};

TEST_F(BPlusTreeTest, BPlusTreeTest1) {
  GenericComparator<24> comparator;
  auto ibpm = new IndexBufferPoolManager(50, fileStore);
  auto ifm = new IndexFileManager(ibpm);
  auto tree = new BPlusTree<GenericKey<24>, RecordIDType , GenericComparator<24>>(ifm, comparator);
  
  // create and fetch header_page
  auto headerPage = ifm->fetchIndexPage(0);
  
  // first, populate index
  std::vector<GenericKey<24>> forInsert;
  std::vector<GenericKey<24>> forDelete;
  size_t sieve = 2;  // divide evenly
  size_t totalKeys = 1000;
  for (size_t i = 1; i <= totalKeys; i++) {
    std::string str = std::to_string(i);
    
    if (i % sieve == 0) {
      forInsert.emplace_back(str);
    } else {
      forDelete.emplace_back(str);
    }
  
    // Insert all the keys, including the ones that will remain at the end and
    // the ones that are going to be removed next.
    tree->insert(GenericKey<24>(str), RecordIDType{i});
  }
  
  // Remove the keys in forDelete
  for (auto key : forDelete) {
    tree->remove(key);
  }
  
  // sort forInsert by lexicographical order using GenericComparator
  std::sort(forInsert.begin(), forInsert.end(), [&](const GenericKey<24> &lhs, GenericKey<24> &rhs) {
    return comparator(lhs, rhs) < 0;
  });
  
  // Only half of the keys should remain
  int64_t size = 0;
  for (auto pair : *tree) {
    EXPECT_EQ(0, strncmp(pair.first.getData(), forInsert[size].getData(), 24));
    size++;
  }
  
  EXPECT_EQ(size, forInsert.size());
  
  ifm->unpinIndexPage(0, true);
  
  delete tree;
  delete ifm;
  delete ibpm;
  delete fileStore;
  
  // test persistence
  
  fileStore = new FileStore(dbName);
  ibpm = new IndexBufferPoolManager(50, fileStore);
  ifm = new IndexFileManager(ibpm);
  tree = new BPlusTree<GenericKey<24>, RecordIDType , GenericComparator<24>>(ifm, comparator);
  
  size = 0;
  for (auto pair : *tree) {
    EXPECT_EQ(0, strncmp(pair.first.getData(), forInsert[size].getData(), 24));
    size++;
  }
  
  EXPECT_EQ(size, forInsert.size());
  
  delete tree;
  delete ifm;
  delete ibpm;
}
}