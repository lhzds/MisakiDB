#include "page/b_plus_tree_leaf_page.h"

namespace MisakiDB {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/

/**
 * init method after creating a new leaf page
 * Including set page type, set current size to zero, set page id/parent id, set
 * next page id and set max size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::init(PageIDType pageID, PageIDType parentID, int maxSize) {
  m_nextPageID = INVALID_PAGE_ID;
  setPageID(pageID);
  setParentPageID(parentID);
  setSize(0);
  setMaxSize(maxSize);
  setPageType(IndexPageType::LEAF_PAGE);
}

/**
 * Helper methods to set/get next page id
 */
INDEX_TEMPLATE_ARGUMENTS
PageIDType B_PLUS_TREE_LEAF_PAGE_TYPE::getNextPageID() const { return m_nextPageID; }

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::setNextPageID(PageIDType nextPageID) { m_nextPageID = nextPageID; }

/**
 * Helper method to find the first index i so that array[i].first >= key
 * NOTE: This method is only used when generating index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::keyIndex(const KeyType &key, const KeyComparator &comparator) const {
  int i = 0;
  for (; i < getSize() && comparator(keyAt(i), key) < 0; ++i) {
  }
  return i;
}

/*
 * Helper method to find and return the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_LEAF_PAGE_TYPE::keyAt(int index) const {
  assert (0 <= index && index < getSize());
  return m_array[index].first;
}

/*
 * Helper method to find and return the key & value pair associated with input
 * "index"(a.k.a array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
const MappingType &B_PLUS_TREE_LEAF_PAGE_TYPE::getItem(int index) {
  assert (0 <= index && index < getSize());
  return m_array[index];
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * insert key & value pair into leaf page ordered by key
 * @return  page size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_LEAF_PAGE_TYPE::insert(const KeyType &key, const ValueType &value, const KeyComparator &comparator) {
  int i = getSize();
  for (; 0 < i && comparator(key, keyAt(i - 1)) < 0; --i) {
    m_array[i] = m_array[i - 1];
  }
  m_array[i] = {key, value};
  increaseSize(1);
  return getSize();
}

/*****************************************************************************
 * SPLIT
 *****************************************************************************/
/*
 * Remove half of key & value pairs from this page to "recipient" page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::moveHalfTo(BPlusTreeLeafPage *recipient) {
  int half = (getSize() + 1) / 2;
  recipient->copyNFrom(m_array + half, getSize() - half);
  setSize(half);
}

/*
 * Copy starting from items, and append {size} number of elements into me.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::copyNFrom(MappingType *items, int size) {
  int oldSize = getSize();
  increaseSize(size);
  for (int i = 0; i < size; ++i) {
    m_array[i + oldSize] = items[i];
  }
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * For the given key, check to see whether it exists in the leaf page. If it
 * does, then store its corresponding value in input "value" and return true.
 * If the key does not exist, then return false
 */
INDEX_TEMPLATE_ARGUMENTS
bool B_PLUS_TREE_LEAF_PAGE_TYPE::lookup(const KeyType &key, ValueType *value, const KeyComparator &comparator) const {
  for (int i = 0; i < getSize(); ++i) {
    if (comparator(keyAt(i), key) == 0) {
      if (value != nullptr) {
        *value = m_array[i].second;
      }
      return true;
    }
  }
  return false;
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * First look through leaf page to see whether delete key exist or not. If
 * existed, perform deletion, otherwise return immediately.
 * NOTE: store key&value pair continuously after deletion
 * @return   removed value
 */
INDEX_TEMPLATE_ARGUMENTS
std::optional<ValueType>
B_PLUS_TREE_LEAF_PAGE_TYPE::removeAndDeleteRecord(const KeyType &key, const KeyComparator &comparator) {
  int i = 0;
  for (; i < getSize() && comparator(keyAt(i), key) != 0; ++i);
  
  // if key not exist, return immediately
  if (i == getSize()) {
    return std::nullopt;
  }
  
  ValueType removedValue = m_array[i].second;
  // delete key and its corresponding value
  for (; i < getSize(); ++i) {
    m_array[i] = m_array[i + 1];
  }
  increaseSize(-1);
  return removedValue;
}

/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all key & value pairs from this page to "recipient" page. Don't forget
 * to update the next_page id in the sibling page
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::moveAllTo(BPlusTreeLeafPage *recipient) {
  recipient->copyNFrom(m_array, getSize());
  setSize(0);
  recipient->setNextPageID(getNextPageID());
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::moveFirstToEndOf(BPlusTreeLeafPage *recipient) {
  recipient->copyLastFrom(m_array[0]);
  int oldSize = getSize();
  for (int i = 1; i < oldSize; ++i) {
    m_array[i - 1] = m_array[i];
  }
  increaseSize(-1);
}

/*
 * Copy the item into the end of my item list. (Append item to my array)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::copyLastFrom(const MappingType &item) {
  int old_size = getSize();
  m_array[old_size] = item;
  increaseSize(1);
}

/*
 * Remove the last key & value pair from this page to "recipient" page.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::moveLastToFrontOf(BPlusTreeLeafPage *recipient) {
  int oldSize = getSize();
  recipient->copyFirstFrom(m_array[oldSize - 1]);
  increaseSize(-1);
}

/*
 * insert item at the front of my items. Move items accordingly.
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_LEAF_PAGE_TYPE::copyFirstFrom(const MappingType &item) {
  int oldSize = getSize();
  for (int i = oldSize; i > 0; --i) {
    m_array[i] = m_array[i - 1];
  }
  m_array[0] = item;
  increaseSize(1);
}

template class BPlusTreeLeafPage<GenericKey<RECORD_KEY_SIZE>, RecordIDType, GenericComparator<RECORD_KEY_SIZE>>;
}

