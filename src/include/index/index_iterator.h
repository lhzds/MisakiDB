#pragma once
#include "page/b_plus_tree_leaf_page.h"
#include "file_manager/index_file_manager.h"

namespace MisakiDB {
#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

/**
 * index_iterator.h
 * For range scan of b+ tree
 */
INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

public:
  IndexIterator(LeafPage *beginLeafPtr, IndexFileManager *indexFileManager, int beginItemIndex = 0);
  ~IndexIterator();
  
  bool isEnd();
  
  const MappingType &operator *();
  
  IndexIterator &operator ++();
  
  bool operator ==(const IndexIterator &rhs) const {
    return m_curLeafPtr == rhs.m_curLeafPtr && m_curItemIndex == rhs.m_curItemIndex;
  }
  
  bool operator!=(const IndexIterator &rhs) const { return !operator ==(rhs); }

private:
  void nextPage_helper();
  
  LeafPage *m_curLeafPtr;
  int m_curItemIndex;
  IndexFileManager *m_indexFileManager;
};

}