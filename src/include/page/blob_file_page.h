#pragma once
#include "globals.h"

namespace MisakiDB {
class BlobFilePage {
public:
  void init();
  
  PageIDType getBlobListNextPageID() const;
  void setBlobListNextPageID(PageIDType blobListNextPageID);
  int getDataSize() const;
  void setDataSize(int dataSize);
  
private:
  PageIDType m_blobListNextPageID;
  int m_dataSize;
  char m_data[0];
};
}