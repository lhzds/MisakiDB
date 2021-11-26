#pragma once
#include "globals.h"
#include "page/b_plus_tree_leaf_page.h"
#include "page/b_plus_tree_internal_page.h"
#include "file_manager/index_file_manager.h"
#include "index/index_iterator.h"

namespace MisakiDB {

#define BPLUSTREE_TYPE BPlusTree<KeyType, ValueType, KeyComparator>

/**
 * Main class providing the API for the Interactive B+ Tree.
 *
 * Implementation of simple b+ tree data structure where internal pages direct
 * the search and leaf pages contain actual data.
 * (1) We only support unique key
 * (2) support insert & remove
 * (3) The structure should shrink and grow dynamically
 * (4) Implement index iterator for range scan
 */
INDEX_TEMPLATE_ARGUMENTS
class BPlusTree {
  using InternalPage = BPlusTreeInternalPage<KeyType, PageIDType, KeyComparator>;
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

public:
  explicit BPlusTree(IndexFileManager *indexFileManager, const KeyComparator &comparator,
                     int leafMaxSize = LEAF_PAGE_SIZE, int internalMaxSize = INTERNAL_PAGE_SIZE);
  
  // Returns true if this B+ tree has no keys and values.
  bool isEmpty() const;
  
  // insert a key-value pair into this B+ tree.
  bool insert(const KeyType &key, const ValueType &value);
  
  // Remove a key and its value from this B+ tree.
  bool remove(const KeyType &key);
  
  // return the value associated with a given key
  bool getValue(const KeyType &key, std::vector<ValueType> *result);
  
  // index iterator
  INDEXITERATOR_TYPE begin();
  INDEXITERATOR_TYPE begin(const KeyType &key);
  INDEXITERATOR_TYPE end();

private:
  Page *findLeafPage(const KeyType &key, bool leftMost = false);
  
  void startNewTree(const KeyType &key, const ValueType &value);
  
  bool insertIntoLeaf(const KeyType &key, const ValueType &value);
  
  void insertIntoParent(BPlusTreePage *oldNode, const KeyType &key, BPlusTreePage *newNode);
  
  template <typename N>
  N *split(N *node);
  
  template <typename N>
  bool coalesceOrRedistribute(N *node);
  
  template <typename N>
  bool coalesce(N **neighborNode, N **node, BPlusTreeInternalPage<KeyType, PageIDType , KeyComparator> **parent, int index);
  
  template <typename N>
  void redistribute(N *neighborNode, N *node, int index);
  
  bool adjustRoot(BPlusTreePage *node);
  
  void updateRootPageID();
  
  // member variable
  PageIDType m_rootPageID;
  IndexFileManager *m_indexFileManager;
  KeyComparator m_comparator;
  int m_leafMaxSize;
  int m_internalMaxSize;
};

}


