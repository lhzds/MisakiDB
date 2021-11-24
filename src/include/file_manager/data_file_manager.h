#pragma once
#include "globals.h"
#include "buffer/data_buffer_pool_manager.h"

namespace MisakiDB {
class DataFileManager {
public:
  DataFileManager(DataBufferPoolManager *dataBufferPoolManager);
  
  Page *fetchDataPage(PageIDType pageID);
  bool unpinDataPage(PageIDType pageID, bool isDirty);
  bool flushDataPage(PageIDType pageID);
  
  Page *allocateDataPage(RecordSizeType recordSize);
  void addFreeSpace(PageIDType pageID, RecordSizeType recordSize);

private:
  static PageIDType convertToDataPageID(PageIDType mapPageID, int mapIndex);
  static PageIDType convertToMapPageID(PageIDType dataPageID);
  static int convertToMapIndex(PageIDType dataPageID);
  
  DataBufferPoolManager *m_dataBufferPoolManager;
};
}

