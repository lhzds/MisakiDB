#include "file_store/file_store.h"
#include "fileapi.h"

namespace MisakiDB{
FileStore::FileStore(const std::string &dbName) {
  std::string indexFileName {dbName + "\\idx.mdb"};
  std::string dataFileName { dbName + "\\dt.mdb"};
  std::string freeSpaceMapFileName {dbName + "\\fs_map.mdb"};
  
  bool needInitFileStruct = false;
  if ((needInitFileStruct = CreateDirectoryA(dbName.c_str(), NULL))) {
    createFile(indexFileName);
    createFile(dataFileName);
    createFile(freeSpaceMapFileName);
  }
  
  m_indexFileIO.open(indexFileName, std::ios::binary | std::ios::in | std::ios::out);
  if (!m_indexFileIO.is_open()) {
    throw std::runtime_error("fail to open index file");
  }
  m_dataFileIO.open(dataFileName, std::ios::binary | std::ios::in | std::ios::out);
  if (!m_dataFileIO.is_open()) {
    throw std::runtime_error("fail to open data file");
  }
  m_freeSpaceMapFileIO.open(freeSpaceMapFileName, std::ios::binary | std::ios::in | std::ios::out);
  if (!m_freeSpaceMapFileIO.is_open()) {
    throw std::runtime_error("fail to open free-space map file");
  }
  
  if (needInitFileStruct) {
    initFileStruct();
  }
}

FileStore::~FileStore() {
  m_indexFileIO.close();
  m_dataFileIO.close();
  m_freeSpaceMapFileIO.close();
}

FileSizeType FileStore::pageIDToOffset(PageIDType pageID) {
  // pageID * 4096
  return pageID << 12;
}

void FileStore::readRawPage(FILE_TYPE fileType, PageIDType pageID, ByteType *raw) {
  switch (fileType) {
  case FILE_TYPE::INDEX:
    readRawPage_Helper(m_indexFileIO, pageID, raw, "index file");
    break;
  case FILE_TYPE::DATA:
    readRawPage_Helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FILE_TYPE::FREE_SPACE_MAP:
    readRawPage_Helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::writeRawPage(FILE_TYPE fileType, PageIDType pageID, const ByteType *raw) {
  switch (fileType) {
  case FILE_TYPE::INDEX:
    writeRawPage_Helper(m_indexFileIO, pageID, raw, "index file");
    break;
  case FILE_TYPE::DATA:
    writeRawPage_Helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FILE_TYPE::FREE_SPACE_MAP:
    writeRawPage_Helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::readRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                   ByteType *raw, const std::string &fileName) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  fileIO.seekg(offset, std::ios::beg);
  fileIO.read(raw, PAGE_SIZE);
  if (fileIO.bad()) {
    throw std::runtime_error("IO error while reading " + fileName);
  }
  
  int read_bits = fileIO.gcount();
  if (read_bits < PAGE_SIZE) {
    throw std::runtime_error("read less than one page while reading " + fileName);
  }
  
}

void FileStore::writeRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                    const ByteType *raw, const std::string &fileName) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  fileIO.seekp(offset, std::ios::beg);
  fileIO.write(raw, PAGE_SIZE);
  if (fileIO.bad()) {
    throw std::runtime_error("IO error while writing " + fileName);
  }
  
  fileIO.flush();
}

void FileStore::createFile(const std::string &fileName) {
  std::fstream fs{fileName, std::ios::out};
  fs.close();
}

void FileStore::initFileStruct() {
  // init index file header
  ByteType raw1[PAGE_SIZE]{};
  IndexFileHeader initIndexFileHeader;
  memcpy(raw1, &initIndexFileHeader, sizeof(IndexFileHeader));
  writeRawPage_Helper(m_indexFileIO, 0, raw1, "index file");
  
  // init free-space map file header
  ByteType raw2[PAGE_SIZE]{};
  FreeSpaceMapFileHeader initFreeSpaceMapFileHeader;
  memcpy(raw2, &initFreeSpaceMapFileHeader, sizeof(FreeSpaceMapFileHeader));
  writeRawPage_Helper(m_freeSpaceMapFileIO, 0, raw2, "free-space map file");
}
}
