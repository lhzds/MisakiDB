#pragma once

#include "globals.h"
#include "page/b_plus_tree_page.h"

namespace MisakiDB {
#define B_PLUS_TREE_LEAF_PAGE_TYPE BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>
#define LEAF_PAGE_HEADER_SIZE (sizeof(BPlusTreePage) + sizeof(PageIDType))
#define LEAF_PAGE_SIZE ((PAGE_SIZE - LEAF_PAGE_HEADER_SIZE) / sizeof(MappingType))

/**
 * Store indexed key and record id(record id = page id combined with slot id,
 * see include/common/rid.h for detailed implementation) together within leaf
 * page. Only support unique key.
 *
 * Leaf page format (keys are stored in order):
 *  ----------------------------------------------------------------------
 * | HEADER | KEY(1) + RID(1) | KEY(2) + RID(2) | ... | KEY(n) + RID(n)
 *  ----------------------------------------------------------------------
 */
INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeLeafPage : public BPlusTreePage {
public:
  // After creating a new leaf page from buffer pool, must call initialize
  // method to set default values
  void init(PageIDType pageID, PageIDType parentID = INVALID_PAGE_ID, int maxSize = LEAF_PAGE_SIZE);
  // helper methods
  PageIDType getNextPageID() const;
  void setNextPageID(PageIDType nextPageID);
  KeyType keyAt(int index) const;
  int keyIndex(const KeyType &key, const KeyComparator &comparator) const;
  const MappingType &getItem(int index);
  
  // insert and delete methods
  int insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator);
  bool lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const;
  int removeAndDeleteRecord(const KeyType &key, const KeyComparator &comparator);
  
  // Split and Merge utility methods
  void moveHalfTo(BPlusTreeLeafPage *recipient);
  void moveAllTo(BPlusTreeLeafPage *recipient);
  void moveFirstToEndOf(BPlusTreeLeafPage *recipient);
  void moveLastToFrontOf(BPlusTreeLeafPage *recipient);

private:
  void copyNFrom(MappingType *items, int size);
  void copyLastFrom(const MappingType &item);
  void copyFirstFrom(const MappingType &item);
  PageIDType m_nextPageID;
  MappingType m_array[0];
};
}
