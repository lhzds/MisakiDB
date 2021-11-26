#include "page/b_plus_tree_internal_page.h"

namespace MisakiDB {

/*****************************************************************************
 * HELPER METHODS AND UTILITIES
 *****************************************************************************/
/*
 * Init method after creating a new internal page
 * Including set page type, set current size, set page id, set parent id and set
 * max page size
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::init(PageIDType pageID, PageIDType parentID, int maxSize) {
  setPageID(pageID);
  setParentPageID(parentID);
  setSize(0);
  
  setPageType(IndexPageType::INTERNAL_PAGE);
  // calculate max with 4 cuz GetSize() include the
  // first key-value pair whose key is invalid, and
  // an internal node should have at least 3 pointers.
  // (otherwise, the new node will own only one pointer after splitting)
  setMaxSize(std::max(maxSize, 4));
}

/*
 * Helper method to get/set the key associated with input "index"(a.k.a
 * array offset)
 */
INDEX_TEMPLATE_ARGUMENTS
KeyType B_PLUS_TREE_INTERNAL_PAGE_TYPE::keyAt(int index) const {
  assert (0 <= index && index < getSize());
  return m_array[index].first;
}

INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::setKeyAt(int index, const KeyType &key) {
  assert (0 <= index && index < getSize());
  m_array[index].first = key;
}

/*
 * Helper method to find and return array index(or offset), so that its value
 * equals to input "value"
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::valueIndex(const ValueType &value) const {
  for (int i = 0; i < getSize(); ++i) {
    if (valueAt(i) == value) {
      return i;
    }
  }
  throw std::runtime_error("value doesn't exist");
}

/*
 * Helper method to get the value associated with input "index"(a.k.a array
 * offset)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::valueAt(int index) const {
  assert (0 <= index && index < getSize());
  return m_array[index].second;
}

/*****************************************************************************
 * LOOKUP
 *****************************************************************************/
/*
 * Find and return the child pointer(page_id) which points to the child page
 * that contains input "key"
 * Start the search from the second key(the first key should always be invalid)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::lookup(const KeyType &key, const KeyComparator &comparator) const {
  int i;
  for (i = 1; i < getSize(); ++i) {
    if (comparator(key, keyAt(i)) < 0) {
      break;
    }
  }
  return valueAt(i - 1);
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * Populate new root page with old_value + new_key & new_value
 * When the insertion cause overflow from leaf page all the way upto the root
 * page, you should create a new root page and populate its elements.
 * NOTE: This method is only called within InsertIntoParent()(b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::populateNewRoot(const ValueType &oldValue, const KeyType &newKey,
                                                     const ValueType &newValue) {
  assert (isRootPage() && getSize() == 0);
  m_array[0].second = oldValue;
  m_array[1] = {newKey, newValue};
  setSize(2);
}
/*
 * insert new_key & new_value pair right after the pair with its value ==
 * old_value
 * @return:  new size after insertion
 */
INDEX_TEMPLATE_ARGUMENTS
int B_PLUS_TREE_INTERNAL_PAGE_TYPE::insertNodeAfter(const ValueType &oldValue, const KeyType &newKey,
                                                    const ValueType &newValue) {
  int i;
  for (i = getSize() - 1; 0 <= i && valueAt(i) != oldValue; --i) {
    m_array[i + 1] = m_array[i];
  }
  assert (0 <= i); // oldValue must exist
  m_array[i + 1] = {newKey, newValue};
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
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::moveHalfTo(BPlusTreeInternalPage *recipient,
                                                IndexFileManager *indexFileManager) {
  int half = (getSize() + 1) / 2;
  recipient->copyNFrom(m_array + half, getSize() - half, indexFileManager);
  setSize(half);
}

/* Append entries into me, starting from {items} and copy {size} entries.
 * Since it is an internal page, for all entries (pages) moved, their parents page now changes to me.
 * So I need to 'adopt' them by changing their parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::copyNFrom(MappingType *items, int size, IndexFileManager *indexFileManager) {
  int oldSize = getSize();
  increaseSize(size);
  for (int i = 0; i < size; ++i) {
    m_array[i + oldSize] = items[i];
    Page *rawPage = indexFileManager->fetchIndexPage(valueAt(i));
    auto bpTreePagePtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
    bpTreePagePtr->setParentPageID(getPageID());
    indexFileManager->unpinIndexPage(valueAt(i), true);
  }
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Remove the key & value pair in internal page according to input index(a.k.a.
 * array offset)
 * NOTE: store key&value pair continuously after deletion
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::remove(int index) {
  assert (0 <= index && index < getSize());
  for (int i = index; i < getSize() - 1; ++i) {
    m_array[i] = m_array[i + 1];
  }
  increaseSize(-1);
}

/*
 * Remove the only key & value pair in internal page and return the value
 * NOTE: only call this method within AdjustRoot()(in b_plus_tree.cpp)
 */
INDEX_TEMPLATE_ARGUMENTS
ValueType B_PLUS_TREE_INTERNAL_PAGE_TYPE::removeAndReturnOnlyChild() {
  ValueType val = valueAt(0);
  increaseSize(-1);
  return val;
}
/*****************************************************************************
 * MERGE
 *****************************************************************************/
/*
 * Remove all key & value pairs from this page to "recipient" page.
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::moveAllTo(BPlusTreeInternalPage *recipient, const KeyType &middleKey,
                                               IndexFileManager *indexFileManager) {
  setKeyAt(0, middleKey);
  recipient->copyNFrom(m_array, getSize(), indexFileManager);
  setSize(0);
}

/*****************************************************************************
 * REDISTRIBUTE
 *****************************************************************************/
/*
 * Remove the first key & value pair from this page to tail of "recipient" page.
 *
 * The middle_key is the separation key you should get from the parent. You need
 * to make sure the middle key is added to the recipient to maintain the invariant.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those
 * pages that are moved to the recipient
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::moveFirstToEndOf(BPlusTreeInternalPage *recipient, const KeyType &middleKey,
                                                      IndexFileManager *indexFileManager) {
  recipient->copyLastFrom(m_array[0], indexFileManager);
  recipient->setKeyAt(recipient->getSize() - 1, middleKey);
  
  int oldSize = getSize();
  for (int i = 0; i < oldSize - 1; ++i) {
    m_array[i] = m_array[i + 1];
  }
  increaseSize(-1);
}

/* Append an entry at the end.
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::copyLastFrom(const MappingType &pair, IndexFileManager *indexFileManager) {
  increaseSize(1);
  m_array[getSize() - 1] = pair;
  Page *rawPage = indexFileManager->fetchIndexPage(valueAt(getSize() - 1));
  auto bpTreePgPtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
  bpTreePgPtr->setParentPageID(getPageID());
  indexFileManager->unpinIndexPage(bpTreePgPtr->getPageID(), true);
}

/*
 * Remove the last key & value pair from this page to head of "recipient" page.
 * You need to handle the original dummy key properly, e.g. updating recipient’s array to position the middle_key at the
 * right place.
 * You also need to use BufferPoolManager to persist changes to the parent page id for those pages that are
 * moved to the recipient
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::moveLastToFrontOf(BPlusTreeInternalPage *recipient, const KeyType &middleKey,
                                                       IndexFileManager *indexFileManager) {
  recipient->setKeyAt(0, middleKey);
  recipient->copyFirstFrom(m_array[getSize() - 1], indexFileManager);
  increaseSize(-1);
}

/* Append an entry at the beginning.
 * Since it is an internal page, the moved entry(page)'s parent needs to be updated.
 * So I need to 'adopt' it by changing its parent page id, which needs to be persisted with BufferPoolManger
 */
INDEX_TEMPLATE_ARGUMENTS
void B_PLUS_TREE_INTERNAL_PAGE_TYPE::copyFirstFrom(const MappingType &pair, IndexFileManager *indexFileManager) {
  increaseSize(1);
  for (int i = getSize(); 0 < i; --i) {
    m_array[i] = m_array[i - 1];
  }
  
  m_array[0] = pair;
  Page *rawPage = indexFileManager->fetchIndexPage(valueAt(0));
  auto bpTreePgPtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
  bpTreePgPtr->setParentPageID(getPageID());
  indexFileManager->unpinIndexPage(bpTreePgPtr->getPageID(), true);
}

template class BPlusTreeInternalPage<GenericKey<RECORD_KEY_SIZE>, PageIDType, GenericComparator<RECORD_KEY_SIZE>>;
}
