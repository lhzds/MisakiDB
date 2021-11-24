#pragma once

#include "globals.h"

namespace MisakiDB {
#define MappingType std::pair<KeyType, ValueType>

#define INDEX_TEMPLATE_ARGUMENTS template <typename KeyType, typename ValueType, typename KeyComparator>

// define page type enum
enum class IndexPageType { INVALID_INDEX_PAGE = 0, LEAF_PAGE, INTERNAL_PAGE };

/**
 * Both internal and leaf page are inherited from this page.
 *
 * It actually serves as a header part for each B+ tree page and
 * contains information shared by both leaf page and internal page.
 */
class BPlusTreePage {
public:
  bool isLeafPage() const;
  bool isRootPage() const;
  void setPageType(IndexPageType pageType);
  
  int getSize() const;
  void setSize(int size);
  void increaseSize(int amount);
  
  int getMaxSize() const;
  void setMaxSize(int maxSize);
  int getMinSize() const;
  
  PageIDType getParentPageID() const;
  void setParentPageID(PageIDType parentPageID);
  
  PageIDType getPageID() const;
  void setPageID(PageIDType pageID);

private:
  // member variable, attributes that both internal and leaf page share
  IndexPageType m_pageType;
  int m_size;
  int m_maxSize;
  PageIDType m_parentPageID;
  PageIDType m_pageID;
};
}
