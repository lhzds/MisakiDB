#include "page/b_plus_tree_page.h"

namespace MisakiDB {
/*
 * Helper methods to get/set page type.
 * Page type enum class is defined in b_plus_tree_page.h
 */
bool BPlusTreePage::isLeafPage() const { return m_pageType == IndexPageType::LEAF_PAGE; }
bool BPlusTreePage::isRootPage() const { return m_parentPageID == INVALID_PAGE_ID; }
void BPlusTreePage::setPageType(IndexPageType pageType) { m_pageType = pageType; }

/*
 * Helper methods to get/set size (number of key/value pairs stored in that
 * page)
 */
int BPlusTreePage::getSize() const { return m_size; }
void BPlusTreePage::setSize(int size) { m_size = size; }
void BPlusTreePage::increaseSize(int amount) { m_size += amount; }

/*
 * Helper methods to get/set max size (capacity) of the page
 */
int BPlusTreePage::getMaxSize() const { return m_maxSize; }
void BPlusTreePage::setMaxSize(int maxSize) { m_maxSize = maxSize; }

/*
 * Helper method to get min page size
 * Generally, min page size == max page size / 2
 */
int BPlusTreePage::getMinSize() const { return m_maxSize / 2; }

/*
 * Helper methods to get/set parent page id
 */
PageIDType BPlusTreePage::getParentPageID() const { return m_parentPageID; }
void BPlusTreePage::setParentPageID(PageIDType parentPageID) { m_parentPageID = parentPageID; }

/*
 * Helper methods to get/set self page id
 */
PageIDType BPlusTreePage::getPageID() const { return m_pageID; }
void BPlusTreePage::setPageID(PageIDType pageID) { m_pageID = pageID; }
}