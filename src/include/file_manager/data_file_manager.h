#pragma once
#include "globals.h"
#include "buffer/data_buffer_pool_manager.h"

namespace MisakiDB {
class DataFileManager {
public:
  explicit DataFileManager(DataBufferPoolManager *dataBufferPoolManager);
  
  // data file
  Page *fetchDataPage(PageIDType pageID);
  bool unpinDataPage(PageIDType pageID, bool isDirty);
  
  // free-space map file
  Page *allocateDataPage(RecordSizeType recordSize);
  void addFreeSpace(PageIDType pageID, RecordSizeType recordSize);
  
  // blob file
  Page *fetchBlobPage(PageIDType pageID);
  bool unpinBlobPage(PageIDType pageID, bool isDirty);
  Page *newBlobPage();
  bool deleteBlobPage(PageIDType pageID);
  
private:
  inline static PageIDType convertToDataPageID(PageIDType mapPageID, int mapIndex);
  inline static PageIDType convertToMapPageID(PageIDType dataPageID);
  inline static int convertToMapIndex(PageIDType dataPageID);
  
  DataBufferPoolManager *m_dataBufferPoolManager;
};
}

