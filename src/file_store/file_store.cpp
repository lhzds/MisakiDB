#include "file_store/file_store.h"
#include "fileapi.h"

namespace MisakiDB{
FileSizeType FileStore::pageIDToOffset(PageIDType pageID) {
  // pageID * 4096
  return pageID << 12;
}
}
