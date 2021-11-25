#include "file_store/file_store.h"
#include "fileapi.h"

namespace MisakiDB{
FileStore::FileStore(const std::string &dbName) {
  std::string indexFileName {dbName + "\\idx.mdb"};
  std::string dataFileName { dbName + "\\dt.mdb"};
  std::string freeSpaceMapFileName {dbName + "\\fs_map.mdb"};
  std::string blobFileName {dbName + "\\blob.mdb"};
  
  bool needInitFileStruct = false;
  if ((needInitFileStruct = CreateDirectoryA(dbName.c_str(), NULL))) {
    createFile(indexFileName);
    createFile(dataFileName);
    createFile(freeSpaceMapFileName);
    createFile(blobFileName);
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
  m_blobFileIO.open(blobFileName, std::ios::binary | std::ios::in | std::ios::out);
  if (!m_blobFileIO.is_open()) {
    throw std::runtime_error("fail to open blob file");
  }
  
  if (needInitFileStruct) {
    initFileStruct();
  }
}

FileStore::~FileStore() {
  m_indexFileIO.close();
  m_dataFileIO.close();
  m_freeSpaceMapFileIO.close();
  m_blobFileIO.close();
}

FileSizeType FileStore::pageIDToOffset(PageIDType pageID) {
  // pageID * 4096
  return pageID << 12;
}

void FileStore::readRawPage(FileType fileType, PageIDType pageID, ByteType *raw) {
  switch (fileType) {
  case FileType::INDEX:
    readRawPage_helper(m_indexFileIO, pageID, raw, "index file");
    break;
  case FileType::DATA:
    readRawPage_helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FileType::FREE_SPACE_MAP:
    readRawPage_helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map file");
    break;
  case FileType::BLOB:
    readRawPage_helper(m_blobFileIO, pageID, raw, "blob file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::writeRawPage(FileType fileType, PageIDType pageID, const ByteType *raw) {
  switch (fileType) {
  case FileType::INDEX:
    writeRawPage_helper(m_indexFileIO, pageID, raw, "index file");
    break;
  case FileType::DATA:
    writeRawPage_helper(m_dataFileIO, pageID, raw, "data file");
    break;
  case FileType::FREE_SPACE_MAP:
    writeRawPage_helper(m_freeSpaceMapFileIO, pageID, raw, "free-space map file");
    break;
  case FileType::BLOB:
    writeRawPage_helper(m_blobFileIO, pageID, raw, "blob file");
    break;
  default:
    throw std::runtime_error("wrong file type");
    break;
  }
}

void FileStore::readRawPage_helper(std::fstream &fileIO, PageIDType pageID,
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

void FileStore::writeRawPage_helper(std::fstream &fileIO, PageIDType pageID,
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
  auto initIndexFileHeader = reinterpret_cast<IndexFileHeader *>(raw1);
  initIndexFileHeader->init();
  writeRawPage_helper(m_indexFileIO, 0, raw1, "index file");
  
  // init free-space map file header
  ByteType raw2[PAGE_SIZE]{};
  auto initFreeSpaceMapFileHeader = reinterpret_cast<FreeSpaceMapFileHeader *>(raw2);
  initFreeSpaceMapFileHeader->init();
  writeRawPage_helper(m_freeSpaceMapFileIO, 0, raw2, "free-space map file");
  
  // init data file header
  ByteType raw3[PAGE_SIZE]{};
  auto initDataFileHeader = reinterpret_cast<DataFileHeader *>(raw3);
  initDataFileHeader->init();
  writeRawPage_helper(m_dataFileIO, 0, raw3, "data file");
  
  // init blob file header
  ByteType raw4[PAGE_SIZE]{};
  auto initBlobFileHeader = reinterpret_cast<BlobFileHeader *>(raw4);
  initBlobFileHeader->init();
  writeRawPage_helper(m_blobFileIO, 0, raw4, "blob file");
}
}
