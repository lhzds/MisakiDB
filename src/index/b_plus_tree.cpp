#include "index/b_plus_tree.h"

namespace MisakiDB {
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(IndexFileManager *indexFileManager, const KeyComparator &comparator,
                          int leafMaxSize, int internalMaxSize)
    : m_indexFileManager(indexFileManager),
      m_comparator(comparator),
      m_leafMaxSize(leafMaxSize),
      m_internalMaxSize(internalMaxSize) {
  auto indexFileHeader = reinterpret_cast<IndexFileHeader *>(m_indexFileManager->fetchIndexPage(0)->getData());
  m_rootPageID = indexFileHeader->getRootPageID();
  m_indexFileManager->unpinIndexPage(0, false);
}

/*
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::isEmpty() const {
  return m_rootPageID == INVALID_PAGE_ID;
}

/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/*
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::getValue(const KeyType &key, std::vector<ValueType> *result) {
  if (isEmpty()) {
    return false;
  }
  
  auto leafPtr = reinterpret_cast<LeafPage *>(findLeafPage(key, false)->getData());
  
  ValueType val;
  bool keyExist = leafPtr->lookup(key, &val, m_comparator);
  m_indexFileManager->unpinIndexPage(leafPtr->getPageID(), false);
  if (keyExist && result != nullptr) {
    result->push_back(val);
  }
  return keyExist;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/*
 * insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::insert(const KeyType &key, const ValueType &value) {
  if (isEmpty()) {
    startNewTree(key, value);
    return true;
  }
  return insertIntoLeaf(key, value);
}

/*
 * insert constant key & value pair into an empty tree
 * User needs to first ask for new page from file manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then update b+
 * tree's root page id and insert entry directly into leaf page.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::startNewTree(const KeyType &key, const ValueType &value) {
  Page *rawPage = m_indexFileManager->newIndexPage();
  if (rawPage == nullptr) {
    throw std::runtime_error("out of memory");
  }
  
  PageIDType newRootPgID = rawPage->getPageID();
  auto rootPtr = reinterpret_cast<LeafPage *>(rawPage->getData());
  rootPtr->init(newRootPgID, INVALID_PAGE_ID, m_leafMaxSize);
  rootPtr->insert(key, value, m_comparator);
  m_indexFileManager->unpinIndexPage(newRootPgID, true);
  
  // update root page id
  m_rootPageID = newRootPgID;
  updateRootPageID();
}

/*
 * insert constant key & value pair into leaf page
 * User needs to first find the right leaf page as insertion target, then look
 * through leaf page to see whether insert key exist or not. If existed, return
 * immediately, otherwise insert entry. Remember to deal with split if necessary.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::insertIntoLeaf(const KeyType &key, const ValueType &value) {
  auto leafPtr = reinterpret_cast<LeafPage *>(findLeafPage(key, false)->getData());
  // if key exists, return false
  if (leafPtr->lookup(key, nullptr, m_comparator)) {
    return false;
  }
  
  // get new size after insert new key-value pair
  int sz = leafPtr->insert(key, value, m_comparator);
  
  // if leaf's new size greater than or equal the max size, split it
  if (leafPtr->getMaxSize() <= sz) {
    LeafPage *newLeafPtr = split(leafPtr);
    
    // insert new node into leaf nodes list
    newLeafPtr->setNextPageID(leafPtr->getNextPageID());
    leafPtr->setNextPageID(newLeafPtr->getPageID());
    
    insertIntoParent(leafPtr, newLeafPtr->keyAt(0), newLeafPtr);
    m_indexFileManager->unpinIndexPage(newLeafPtr->getPageID(), true);
  }
  
  m_indexFileManager->unpinIndexPage(leafPtr->getPageID(), true);
  return true;
}

/*
 * Split input page and return newly created page.
 * Using template N to represent either internal page or leaf page.
 * User needs to first ask for new page from file manager(NOTICE: throw
 * an "out of memory" exception if returned value is nullptr), then move half
 * of key & value pairs from input page to newly created page
 *
 * @return: the new node split from the original node (NOTICE: the corresponding page
 * of the new node should be unpinned outside when finish operations about it)
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
N *BPLUSTREE_TYPE::split(N *node) {
  if (node->getSize() < node->getMaxSize()) {
    throw std::runtime_error("size of node less than its max size");
  }
  
  Page *newPage = m_indexFileManager->newIndexPage();
  if (newPage == nullptr) {
    throw std::runtime_error("out of memory");
  }
  PageIDType newPageID = newPage->getPageID();
  
  N *newNode = reinterpret_cast<N *>(newPage->getData());
  if constexpr (std::is_same<N, LeafPage>::value) {
    newNode->init(newPageID, node->getParentPageID(), m_leafMaxSize);
    node->moveHalfTo(newNode);
  } else {
    newNode->init(newPageID, node->getParentPageID(), m_internalMaxSize);
    node->moveHalfTo(newNode, m_indexFileManager);
  }
  return newNode;
}

/*
 * insert key & value pair into internal page after split
 * @param   old_node      input page from split() method
 * @param   key
 * @param   new_node      returned page from split() method
 * User needs to first find the parent page of old_node, parent node must be
 * adjusted to take info of new_node into account. Remember to deal with split
 * recursively if necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::insertIntoParent(BPlusTreePage *oldNode, const KeyType &key, BPlusTreePage *newNode) {
  PageIDType oldValue = oldNode->getPageID();
  PageIDType newValue = newNode->getPageID();
  
  // when split spreads to root, create a new root node and populate it
  if (oldNode->isRootPage()) {
    Page *rawPage = m_indexFileManager->newIndexPage();
    PageIDType newRootPgID = rawPage->getPageID();
    auto newRootPtr = reinterpret_cast<InternalPage *>(rawPage->getData());
    newRootPtr->init(newRootPgID, INVALID_PAGE_ID, m_internalMaxSize);
    newRootPtr->populateNewRoot(oldValue, key, newValue);
    
    oldNode->setParentPageID(newRootPgID);
    newNode->setParentPageID(newRootPgID);
  
    m_rootPageID = newRootPgID;
    updateRootPageID();
    
    m_indexFileManager->unpinIndexPage(newRootPgID, true);
    return;
  }
  
  // insert new key-value pair into parent node
  Page *rawPage = m_indexFileManager->fetchIndexPage(oldNode->getParentPageID());
  auto internalPtr = reinterpret_cast<InternalPage *>(rawPage->getData());
  internalPtr->insertNodeAfter(oldValue, key, newValue);
  
  // split the node
  if (internalPtr->getMaxSize() <= internalPtr->getSize()) {
    InternalPage *newInternalPtr = split(internalPtr);

    insertIntoParent(internalPtr, newInternalPtr->keyAt(0), newInternalPtr);
    m_indexFileManager->unpinIndexPage(newInternalPtr->getPageID(), true);
  }
  
  m_indexFileManager->unpinIndexPage(internalPtr->getPageID(), true);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/*
 * Delete key & value pair associated with input key
 * If current tree is empty, return immediately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 * @return: return false if the key to be deleted does not exist,
 * otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::remove(const KeyType &key) {
  if (isEmpty()) {
    return;
  }
  
  auto leafPtr = reinterpret_cast<LeafPage *>(findLeafPage(key, false)->getData());
  auto oldSize = leafPtr->getSize();
  leafPtr->removeAndDeleteRecord(key, m_comparator);
  auto newSize = leafPtr->getSize();
  if (oldSize == newSize) {
    m_indexFileManager->unpinIndexPage(leafPtr->getPageID(), false);
    return false;
  }
  
  bool needDelete = false;
  if (newSize < leafPtr->getMinSize()) {
    needDelete = coalesceOrRedistribute(leafPtr);
  }
  m_indexFileManager->unpinIndexPage(leafPtr->getPageID(), true);
  if (needDelete) {
    m_indexFileManager->deleteIndexPage(leafPtr->getPageID());
  }
  return true;
}

/*
 * User needs to first find the sibling of input page. If sibling's size + input
 * page's size > page's max size, then redistribute. Otherwise, merge.
 * Using template N to represent either internal page or leaf page.
 * @return: true means target leaf page should be deleted, false means no
 * deletion happens
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREE_TYPE::coalesceOrRedistribute(N *node) {
  if (node->isRootPage()) {
    if (node->isLeafPage() && node->getSize() == 0) {
      // if the tree have no element
      adjustRoot(node);
      return true;
    }
    if (!node->isLeafPage() && node->getSize() == 1) {
      // if the root page is internal page and has only one remaining child
      adjustRoot(node);
      return true;
    }

    return false;
  }
  
  // get the parent page of this node
  Page *rawParentPage = m_indexFileManager->fetchIndexPage(node->getParentPageID());
  auto parentPtr = reinterpret_cast<InternalPage *>(rawParentPage->getData());
  
  // get the sibling index in the kv pairs array of parent
  // and the index of middle key between this node's pointer and sibling's pointer from parent's array
  int thisIndex = parentPtr->valueIndex(node->getPageID());
  int siblingIndex;
  int middleIndex;
  if (thisIndex + 1 == parentPtr->getSize()) {
    siblingIndex = thisIndex - 1;
    middleIndex = thisIndex;
  } else {
    siblingIndex = thisIndex + 1;
    middleIndex = siblingIndex;
  }
  
  // get the sibling page
  Page *rawSiblingPage = m_indexFileManager->fetchIndexPage(parentPtr->valueAt(siblingIndex));
  N *siblingPtr = reinterpret_cast<N *>(rawSiblingPage->getData());
  
  if (siblingPtr->getSize() + node->getSize() < node->getMaxSize()) {
    auto oldSiblingPtr = siblingPtr;
    bool needDelete = coalesce(&siblingPtr, &node, &parentPtr, middleIndex);
    m_indexFileManager->unpinIndexPage(parentPtr->getPageID(), true);
    if (needDelete) {
      m_indexFileManager->deleteIndexPage(parentPtr->getPageID());
    }
    
    // Coalesce() may swap the value of node and sibling_ptr
    // if swap occurs, CoalesceOrRedistribute() should delete page it fetched by itself
    if (siblingPtr != oldSiblingPtr) {
      m_indexFileManager->unpinIndexPage(node->getPageID(), true);
      m_indexFileManager->deleteIndexPage(node->getPageID());
      return false;
    }
    m_indexFileManager->unpinIndexPage(siblingPtr->getPageID(), true);
    return true;
  }
  
  redistribute(siblingPtr, node, middleIndex);
  m_indexFileManager->unpinIndexPage(siblingPtr->getPageID(), true);
  m_indexFileManager->unpinIndexPage(parentPtr->getPageID(), true);
  return false;
}

/*
 * Move all the key & value pairs from one page to its sibling page, and notify
 * file manager to delete this page. Parent page must be adjusted to
 * take info of deletion into account. Remember to deal with coalesce or
 * redistribute recursively if necessary.
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 * @param   parent             parent page of input "node"
 * @return  true means parent node should be deleted, false means no deletion
 * happened
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
bool BPLUSTREE_TYPE::coalesce(N **neighborNode, N **node,
                              BPlusTreeInternalPage<KeyType, PageIDType, KeyComparator> **parent, int index) {
  // if node is the predecessor of neighbor_node
  if ((*parent)->valueAt(index) == (*neighborNode)->getPageID()) {
    std::swap(*neighborNode, *node);
  }
  
  if constexpr (std::is_same<N, LeafPage>::value) {
    (*node)->moveAllTo(*neighborNode);
  } else {
    (*node)->moveAllTo(*neighborNode, (*parent)->keyAt(index), m_indexFileManager);
  }
  
  (*parent)->remove(index);
  if ((*parent)->getSize() < (*parent)->getMinSize()) {
    return coalesceOrRedistribute(*parent);
  }
  return false;
}

/*
 * Redistribute key & value pairs from one page to its sibling page. If index ==
 * 0, move sibling page's first key & value pair into end of input "node",
 * otherwise move sibling page's last key & value pair into head of input
 * "node".
 * Using template N to represent either internal page or leaf page.
 * @param   neighbor_node      sibling page of input "node"
 * @param   node               input from method coalesceOrRedistribute()
 */
INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREE_TYPE::redistribute(N *neighborNode, N *node, int index) {
  // get the parent page of this node
  Page *rawParentPage = m_indexFileManager->fetchIndexPage(node->getParentPageID());
  auto parent = reinterpret_cast<InternalPage *>(rawParentPage->getData());
  
  if (parent->valueAt(index) == node->getPageID()) {  // if neighbor_node is predecessor of node
    if constexpr (std::is_same<N, LeafPage>::value) {
      neighborNode->moveLastToFrontOf(node);
      parent->setKeyAt(index, node->keyAt(0));
    } else {
      KeyType lastKey = neighborNode->keyAt(neighborNode->getSize() - 1);
      neighborNode->moveLastToFrontOf(node, parent->keyAt(index), m_indexFileManager);
      parent->setKeyAt(index, lastKey);
    }
  } else {  // if node is predecessor of neighbor_node, symmetric to the previous case
    if constexpr (std::is_same<N, LeafPage>::value) {
      neighborNode->moveFirstToEndOf(node);
      parent->setKeyAt(index, neighborNode->keyAt(0));
    } else {
      KeyType firstKey = neighborNode->keyAt(1);
      neighborNode->moveFirstToEndOf(node, parent->keyAt(index), m_indexFileManager);
      parent->setKeyAt(index, firstKey);
    }
  }
}
/*
 * Update root page if necessary
 * NOTE: size of root page can be less than min size and this method is only
 * called within coalesceOrRedistribute() method
 * case 1: when you delete the last element in root page, but root page still
 * has one last child
 * case 2: when you delete the last element in whole b+ tree
 * @return : true means root page should be deleted, false means no deletion
 * happened
 */
INDEX_TEMPLATE_ARGUMENTS
bool BPLUSTREE_TYPE::adjustRoot(BPlusTreePage *oldRootNode) {
  if (!oldRootNode->isRootPage()) {
    throw std::runtime_error("argument should be root node");
  }
  
  if (oldRootNode->isLeafPage() && oldRootNode->getSize() == 0) {
    m_rootPageID = INVALID_PAGE_ID;
    updateRootPageID();
    return true;
  }
  
  if (!oldRootNode->isLeafPage() && oldRootNode->getSize() == 1) {
    PageIDType newRootPageID = static_cast<InternalPage *>(oldRootNode)->removeAndReturnOnlyChild();
    m_rootPageID = newRootPageID;
    updateRootPageID();
    
    Page *rawPage = m_indexFileManager->fetchIndexPage(newRootPageID);
    auto newRootPgPtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
    newRootPgPtr->setParentPageID(INVALID_PAGE_ID);
    m_indexFileManager->unpinIndexPage(newRootPgPtr->getPageID(), true);
    return true;
  }
  
  return false;
}

/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/*
 * Input parameter is void, find the leftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::begin() {
  auto beginLeafPtr = reinterpret_cast<LeafPage *>(findLeafPage(KeyType{}, true)->getData());
  return INDEXITERATOR_TYPE(beginLeafPtr, m_indexFileManager);
}

/*
 * Input parameter is low-key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::begin(const KeyType &key) {
  auto beginLeafPtr = reinterpret_cast<LeafPage *>(findLeafPage(key, false)->getData());
  int beginItemIndex = beginLeafPtr->keyIndex(key, m_comparator);
  return INDEXITERATOR_TYPE(beginLeafPtr, m_indexFileManager, beginItemIndex);
}

/*
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE BPLUSTREE_TYPE::end() { return INDEXITERATOR_TYPE(nullptr, m_indexFileManager); }

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/*
 * Find leaf page containing particular key, if leftMost flag == true, find
 * the left most leaf page
 */
INDEX_TEMPLATE_ARGUMENTS
Page *BPLUSTREE_TYPE::findLeafPage(const KeyType &key, bool leftMost) {
  if (isEmpty()) {
    throw std::runtime_error("tree is empty");
  }
  
  Page *rawPage = m_indexFileManager->fetchIndexPage(m_rootPageID);
  auto bpTreePgPtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
  
  while (!bpTreePgPtr->isLeafPage()) {
    auto internalPtr = static_cast<InternalPage *>(bpTreePgPtr);
    PageIDType childID;
    if (leftMost) {
      childID = internalPtr->valueAt(0);
    } else {
      childID = internalPtr->lookup(key, m_comparator);
    }
    m_indexFileManager->unpinIndexPage(bpTreePgPtr->getPageID(), false);
    rawPage = m_indexFileManager->fetchIndexPage(childID);
    bpTreePgPtr = reinterpret_cast<BPlusTreePage *>(rawPage->getData());
  }
  
  return rawPage;
}

/*
 * Update/insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      default value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::updateRootPageID() {
  auto indexFileHeader = reinterpret_cast<IndexFileHeader *>(m_indexFileManager->fetchIndexPage(0)->getData());
  indexFileHeader->setRootPageID(m_rootPageID);
  m_indexFileManager->unpinIndexPage(0, true);
}

template class BPlusTree<GenericKey<RECORD_KEY_SIZE>, RecordIDType, GenericComparator<RECORD_KEY_SIZE>>;
}
