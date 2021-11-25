#include "page/blob_file_free_page.h"

namespace MisakiDB {
void BlobFileFreePage::init() {
  m_magic = MAGIC_NUM;
  m_nextFreePageID = INVALID_PAGE_ID;
}

bool BlobFileFreePage::isFreePage() const {
  return m_magic == MAGIC_NUM;
}

PageIDType BlobFileFreePage::getNextFreePageID() const {
  return m_nextFreePageID;
}

void BlobFileFreePage::setNextFreePageID(PageIDType nextFreePageID) {
  m_nextFreePageID = nextFreePageID;
}
}