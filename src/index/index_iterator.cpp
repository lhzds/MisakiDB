#include "index/index_iterator.h"

namespace MisakiDB {
INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::IndexIterator(LeafPage *beginLeafPtr, IndexFileManager *indexFileManager,
                                  int beginItemIndex)
: m_curLeafPtr(beginLeafPtr), m_curItemIndex(beginItemIndex), m_indexFileManager(indexFileManager) {
  if (beginLeafPtr == nullptr) {
    m_curItemIndex = 0;
  } else if (beginLeafPtr->getSize() <= beginItemIndex) {
    nextPage_helper();
  }
}

INDEX_TEMPLATE_ARGUMENTS
INDEXITERATOR_TYPE::~IndexIterator() {
  if (m_curLeafPtr != nullptr) {
    m_indexFileManager->unpinIndexPage(m_curLeafPtr->getPageID(), false);
  }
}

INDEX_TEMPLATE_ARGUMENTS
bool INDEXITERATOR_TYPE::isEnd() { return m_curLeafPtr == nullptr; }

INDEX_TEMPLATE_ARGUMENTS
const MappingType &INDEXITERATOR_TYPE::operator*() {
  if (isEnd()) {
    throw std::runtime_error("iterator has reached end");
  }
  return m_curLeafPtr->getItem(m_curItemIndex);
}

INDEX_TEMPLATE_ARGUMENTS
    INDEXITERATOR_TYPE &INDEXITERATOR_TYPE::operator++() {
  if (isEnd()) {
    throw std::runtime_error("iterator has reached end");
  }
  if (m_curLeafPtr->getSize() <= m_curItemIndex + 1) {
    nextPage_helper();
  } else {
    ++m_curItemIndex;
  }
  return *this;
}

INDEX_TEMPLATE_ARGUMENTS
void INDEXITERATOR_TYPE::nextPage_helper() {
  m_curItemIndex = 0;
  auto prevLeafPtr = m_curLeafPtr;
  if (m_curLeafPtr->getNextPageID() == INVALID_PAGE_ID) {
    m_curLeafPtr = nullptr;
  } else {
    Page *rawPage = m_indexFileManager->fetchIndexPage(m_curLeafPtr->getNextPageID());
    m_curLeafPtr = reinterpret_cast<LeafPage *>(rawPage->getData());
  }
  m_indexFileManager->unpinIndexPage(prevLeafPtr->getPageID(), false);
}

template class IndexIterator<GenericKey<24>, RecordIDType, GenericComparator<24>>;
}