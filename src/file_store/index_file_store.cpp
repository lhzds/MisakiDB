#include "file_store/index_file_store.h"

namespace MisakiDB{
IndexFileStore::IndexFileStore(const std::string &dbName) {
  std::string indexFileName {dbName + "\\idx.mdb"};
  m_indexFileIO.open(indexFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
  // if file not exist
  if (!m_indexFileIO.is_open()) {
    m_indexFileIO.clear();
    initFileStruct(dbName);
  
    // reopen the newly created file
    m_indexFileIO.open(indexFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    if (!m_indexFileIO.is_open()) {
      throw std::runtime_error("fail to open index file");
    }
  }
}

IndexFileStore::~IndexFileStore() {
  m_indexFileIO.close();
}

void IndexFileStore::readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) {
  switch(fileType) {
  case FILE_TYPE::INDEX:
    readRawPage_Helper(m_indexFileIO, pageID, raw, "index file");
    break;
  default:
    throw std::runtime_error("wrong file type");
  }
}

void IndexFileStore::writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) {
  
  switch(fileType) {
  case FILE_TYPE::INDEX:
    writeRawPage_Helper(m_indexFileIO, pageID, raw, "index file");
    break;
  default:
    throw std::runtime_error("wrong file type");
  }
}

void IndexFileStore::initFileStruct(const std::string &dbName) {
  std::string indexFileName {dbName + "\\idx.mdb"};
  
  // create new file
  m_indexFileIO.open(indexFileName, std::ios::out);
  
  // init index file header
  ByteType raw[PAGE_SIZE]{};
  IndexFileHeader initIndexFileHeader;
  memcpy(raw, &initIndexFileHeader, sizeof(IndexFileHeader));
  writeRawPage_Helper(m_indexFileIO, 0, raw, "index file");
  m_indexFileIO.close();
}
}


