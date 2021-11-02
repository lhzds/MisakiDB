#include "file_store/file_store.h"
#include "fileapi.h"

namespace MisakiDB{
FileSizeType FileStore::pageIDToOffset(PageIDType pageID) {
  // pageID * 4096
  return pageID << 12;
}

void FileStore::readRawPage_Helper(std::fstream &fileIO, PageIDType pageID,
                                   ByteType *raw, const std::string &fileName) {
  FileSizeType offset = pageIDToOffset(pageID);
  
  fileIO.seekg(offset);
  fileIO.read((char *)raw, PAGE_SIZE);
  if (!fileIO) {
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
  
  fileIO.seekp(offset);
  fileIO.write((char *)raw, PAGE_SIZE);
  
  if (!fileIO) {
    throw std::runtime_error("IO error while writing " + fileName);
  }
}
}
