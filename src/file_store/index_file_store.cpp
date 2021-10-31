#include "file_store/index_file_store.h"

namespace MisakiDB{
IndexFileStore::IndexFileStore(const std::string &dbName)
  :m_indexFilename(dbName + "\\idx.mdb") {
  m_indexFileIO.open(m_indexFilename, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
  // if file not exist
  if (!m_indexFileIO.is_open()) {
    m_indexFileIO.clear();
  
    // create new file
    m_indexFileIO.open(m_indexFilename, std::ios::out);
    m_indexFileIO.close();
  
    // reopen the newly created file
    m_indexFileIO.open(m_indexFilename, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    if (!m_indexFileIO.is_open()) {
      throw "fail to open index file";
    }
  }
}

IndexFileStore::~IndexFileStore() {
  close();
}

void IndexFileStore::getRawPage(PageIDType pageID, ByteType *raw) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  m_indexFileIO.seekg(offset);
  m_indexFileIO.read((char *)raw, PAGE_SIZE);
  if (!m_indexFileIO) {
    throw "IO error while reading index file";
  }
  
  int read_bits = m_indexFileIO.gcount();
  if (read_bits < PAGE_SIZE) {
    throw "read less than one page while reading index file";
  }
}

void IndexFileStore::writeRawPage(PageIDType pageID, const ByteType *raw) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  m_indexFileIO.seekp(offset);
  m_indexFileIO.write((char *)raw, PAGE_SIZE);
  
  if (!m_indexFileIO) {
    throw "IO error while writing index file";
  }
}

void IndexFileStore::close() {
  m_indexFileIO.close();
}
}


