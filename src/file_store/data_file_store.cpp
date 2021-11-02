#include "file_store/data_file_store.h"

namespace MisakiDB{
DataFileStore::DataFileStore(const std::string &dbName) {
  std::string freeSpaceMapFileName {dbName + "\\fs_map.mdb"};
  std::string dataFileName {dbName + "\\dt.mdb"};
  m_freeSpaceMapFileIO.open(freeSpaceMapFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
  // if file not exist
  if (!m_freeSpaceMapFileIO.is_open()) {
    m_freeSpaceMapFileIO.clear();
    initFileStruct(dbName);
    
    // reopen the newly created file
    m_freeSpaceMapFileIO.open(freeSpaceMapFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
    if (!m_freeSpaceMapFileIO.is_open()) {
      throw std::runtime_error("fail to open free-space map file");
    }
  }
  
  m_dataFileIO.open(dataFileName, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
  if (!m_dataFileIO.is_open()) {
    throw std::runtime_error("fail to open data file");
  }
}

DataFileStore::~DataFileStore() {
  m_dataFileIO.close();
  m_freeSpaceMapFileIO.close();
}

void DataFileStore::readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) {
  switch (fileType) {
  case FILE_TYPE::DATA:
    readRawPage_Helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FILE_TYPE::FREE_SPACE_MAP:
    readRawPage_Helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void DataFileStore::writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) {
  switch (fileType) {
  case FILE_TYPE::DATA:
    writeRawPage_Helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FILE_TYPE::FREE_SPACE_MAP:
    writeRawPage_Helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void DataFileStore::initFileStruct(const std::string &dbName) {
  std::string freeSpaceMapFileName {dbName + "\\fs_map.mdb"};
  std::string dataFileName {dbName + "\\dt.mdb"};
  
  // create new free-space map file
  m_freeSpaceMapFileIO.open(freeSpaceMapFileName, std::ios::out);
  
  // init free-space map file header
  ByteType raw[PAGE_SIZE];
  FreeSpaceMapFileHeader initFreeSpaceMapFileHeader;
  memcpy(raw, &initFreeSpaceMapFileHeader, sizeof(FreeSpaceMapFileHeader));
  writeRawPage_Helper(m_freeSpaceMapFileIO, 0, raw, "free-space map file");
  m_freeSpaceMapFileIO.close();
  
  // create new data file
  m_dataFileIO.open(dataFileName, std::ios::out);
  m_dataFileIO.close();
}
}
