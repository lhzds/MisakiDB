#pragma once

#include "globals.h"
#include "page/b_plus_tree_page.h"
#include "buffer/index_buffer_pool_manager.h"

namespace MisakiDB {
#define B_PLUS_TREE_INTERNAL_PAGE_TYPE BPlusTreeInternalPage<KeyType, ValueType, KeyComparator>
#define INTERNAL_PAGE_HEADER_SIZE (sizeof(BPlusTreePage))
#define INTERNAL_PAGE_SIZE ((PAGE_SIZE - INTERNAL_PAGE_HEADER_SIZE) / (sizeof(MappingType)))
/**
 * Store n indexed keys and n+1 child pointers (pageID) within internal page.
 * Pointer PAGE_ID(i) points to a subtree in which all keys K satisfy:
 * K(i) <= K < K(i+1).
 * NOTE: since the number of keys does not equal to number of child pointers,
 * the first key always remains invalid. That is to say, any search/lookup
 * should ignore the first key.
 *
 * Internal page format (keys are stored in increasing order):
 *  --------------------------------------------------------------------------
 * | HEADER | KEY(1)+PAGE_ID(1) | KEY(2)+PAGE_ID(2) | ... | KEY(n)+PAGE_ID(n) |
 *  --------------------------------------------------------------------------
 */
INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeInternalPage : public BPlusTreePage {
public:
  // must call initialize method after "create" a new node
  void init(PageIDType pageID, PageIDType parentID = INVALID_PAGE_ID, int maxSize = INTERNAL_PAGE_SIZE);
  
  KeyType keyAt(int index) const;
  void setKeyAt(int index, const KeyType &key);
  int valueIndex(const ValueType &value) const;
  ValueType valueAt(int index) const;
  
  ValueType lookup(const KeyType &key, const KeyComparator &comparator) const;
  void populateNewRoot(const ValueType &oldValue, const KeyType &newKey, const ValueType &newValue);
  int insertNodeAfter(const ValueType &oldValue, const KeyType &newKey, const ValueType &newValue);
  void remove(int index);
  ValueType removeAndReturnOnlyChild();
  
  // Split and Merge utility methods
  void moveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middleKey, IndexBufferPoolManager *bufferPoolManager);
  void moveHalfTo(BPlusTreeInternalPage *recipient, IndexBufferPoolManager *bufferPoolManager);
  void moveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middleKey,
                        IndexBufferPoolManager *bufferPoolManager);
  void moveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middleKey,
                         IndexBufferPoolManager *bufferPoolManager);

private:
  void copyNFrom(MappingType *items, int size, IndexBufferPoolManager *bufferPoolManager);
  void copyLastFrom(const MappingType &pair, IndexBufferPoolManager *bufferPoolManager);
  void copyFirstFrom(const MappingType &pair, IndexBufferPoolManager *bufferPoolManager);
  MappingType m_array[0];
};
};
