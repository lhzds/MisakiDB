#pragma once
#include "globals.h"

namespace MisakiDB {
class BlobFilePage {
private:
  PageIDType m_blobListNextPageID;
  int m_dataSize;
  char m_data[0];
  
public:
  static constexpr size_t MAX_DATA_SIZE { PAGE_SIZE - sizeof(m_blobListNextPageID) - sizeof(m_dataSize) };
  
  void init();
  
  PageIDType getBlobListNextPageID() const;
  void setBlobListNextPageID(PageIDType blobListNextPageID);
  int getDataSize() const;
  void setDataSize(int dataSize);
  void writeData(const char *data, size_t dataSize);
  std::string readData() const;
};
}