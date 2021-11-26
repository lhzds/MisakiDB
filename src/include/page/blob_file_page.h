#pragma once
#include "globals.h"

namespace MisakiDB {
class BlobFilePage {
private:
  bool m_isBlobListHead;
  PageIDType m_blobListNextPageID;
  BlobIDType m_blobID;
  int m_dataSize;
  
public:
  void init();
  
  bool isBlobListHead() const;
  void setIsBlobListHead(bool isBlobListHead);
  BlobIDType getBlobID() const;
  void setBlobID(BlobIDType blobID);
  PageIDType getBlobListNextPageID() const;
  void setBlobListNextPageID(PageIDType blobListNextPageID);
  int getDataSize() const;
  void setDataSize(int dataSize);
};

class BlobHeadPage:public BlobFilePage {
private:
  GenericKey<RECORD_KEY_SIZE> m_key;
  char m_data[0];
public:
  static constexpr size_t MAX_DATA_SIZE { PAGE_SIZE - sizeof(BlobFilePage) - RECORD_KEY_SIZE };
  
  void writeData(const char *data, size_t dataSize);
  std::string readData() const;
  std::string getKey() const;
  void setKey(const std::string &key);
};

class BlobNonHeadPage:public BlobFilePage {
private:
  char m_data[0];
public:
  static constexpr size_t MAX_DATA_SIZE { PAGE_SIZE - sizeof(BlobFilePage) };
  
  void writeData(const char *data, size_t dataSize);
  std::string readData() const;
};


}