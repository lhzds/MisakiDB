#include "page/index_file_free_page.h"

namespace MisakiDB {
void IndexFileFreePage::init() {
  m_magic = MAGIC_NUM;
  m_nextFreePageID = INVALID_PAGE_ID;
}

bool IndexFileFreePage::isFreePage() const {
  return m_magic == MAGIC_NUM;
}

PageIDType IndexFileFreePage::getNextFreePageID() const {
  return m_nextFreePageID;
}

void IndexFileFreePage::setNextFreePageID(PageIDType nextFreePageID) {
  m_nextFreePageID = nextFreePageID;
}
}