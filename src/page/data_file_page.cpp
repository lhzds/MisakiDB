#include "page/data_file_page.h"

namespace MisakiDB {
void DataFilePage::init() {
  m_recordNum = 0;
  m_invalidSlotNum = 0;
  m_freeSpaceEndOffset = PAGE_SIZE;
}

int DataFilePage::getSlotsNum() const {
  return m_invalidSlotNum + m_recordNum;
}

const DataFilePage::Slot *DataFilePage::getSlotArray() const {
  return reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + MIN_FIXED_SIZE);
}

DataFilePage::Slot *DataFilePage::getSlotArray() {
  return reinterpret_cast<Slot *>(reinterpret_cast<char *>(this) + MIN_FIXED_SIZE);
}

char *DataFilePage::getRecords() {
  return reinterpret_cast<char *>(this) + m_freeSpaceEndOffset;
}

const char *DataFilePage::getRecords() const {
  return reinterpret_cast<const char *>(this) + m_freeSpaceEndOffset;
}

RecordSizeType DataFilePage::getRecordOffset(int slotArrayIndex) const {
  return getSlotArray()[slotArrayIndex].m_offset;
}

void DataFilePage::setRecordOffset(int slotArrayIndex, RecordSizeType offset) {
  getSlotArray()[slotArrayIndex].m_offset = offset;
}

RecordSizeType DataFilePage::getRecordLength(int slotArrayIndex) const {
  return getSlotArray()[slotArrayIndex].m_length;
}

void DataFilePage::setRecordLength(int slotArrayIndex, RecordSizeType length) {
  getSlotArray()[slotArrayIndex].m_length = length;
}

bool DataFilePage::isBlob(int slotArrayIndex) const {
  return getSlotArray()[slotArrayIndex].m_isBlob;
}

void DataFilePage::setIsBlob(int slotArrayIndex, bool isBlob) {
  getSlotArray()[slotArrayIndex].m_isBlob = isBlob;
}

std::string DataFilePage::getRecord_helper(int slotArrayIndex) const {
  return std::string(reinterpret_cast<const char *>(this) + getRecordOffset(slotArrayIndex),
                     getRecordLength(slotArrayIndex));
}

std::optional<std::pair<std::string, bool>>
DataFilePage::getRecordValue(const std::string &key, int slotArrayIndex) const {
  assert(0 <= slotArrayIndex && slotArrayIndex < getSlotsNum());
  if (getRecordOffset(slotArrayIndex) == INVALID_OFFSET) {
    return std::nullopt;
  }
  std::string record = getRecord_helper(slotArrayIndex);
  std::string_view recordKey(record.c_str(), key.size());
  if (recordKey != key) {
    return std::nullopt;
  }
  std::string_view recordValue(record.c_str() + key.size());
  return std::make_pair(std::string(recordValue), isBlob(slotArrayIndex));
}

std::variant<RecordSizeType, std::string> DataFilePage::removeRecord(int slotArrayIndex) {
  assert(0 <= slotArrayIndex && slotArrayIndex < getSlotsNum());
  assert(getRecordOffset(slotArrayIndex) != INVALID_OFFSET);
  
  std::string record;
  bool isBlob = this->isBlob(slotArrayIndex);
  if (isBlob) {
    record = getRecord_helper(slotArrayIndex);
  }
  RecordSizeType recordLength = getRecordLength(slotArrayIndex);
  char *records = getRecords();
  char *newRecords = records + recordLength;
  RecordSizeType movedSize = getRecordOffset(slotArrayIndex) - m_freeSpaceEndOffset;
  for (int i = 0; i < getSlotsNum(); ++i) {
    auto offset = getRecordOffset(i);
    if (offset != INVALID_OFFSET && m_freeSpaceEndOffset <= offset && offset < m_freeSpaceEndOffset + movedSize) {
      setRecordOffset(i, offset + recordLength);
    }
  }
  memmove(newRecords, records, movedSize);
  m_freeSpaceEndOffset += recordLength;
  setRecordOffset(slotArrayIndex, INVALID_OFFSET);
  --m_recordNum;
  ++m_invalidSlotNum;
  if (isBlob) {
    return record;
  } else {
    return recordLength;
  }
}

RecordSizeType DataFilePage::insertRecord(const std::string& record, bool isBlob) {
  int targetIndex;
  if (m_invalidSlotNum != 0) {
    for (int i = 0; i < getSlotsNum(); ++i) {
      if (getRecordOffset(i) == INVALID_OFFSET) {
        targetIndex = i;
        break;
      }
    }
    --m_invalidSlotNum;
  } else {
    targetIndex = getSlotsNum();
  }
  ++m_recordNum;
  m_freeSpaceEndOffset -= record.size();
  setRecordOffset(targetIndex, m_freeSpaceEndOffset);
  setRecordLength(targetIndex, record.size());
  setIsBlob(targetIndex, isBlob);
  char *records = getRecords();
  memcpy(records, record.c_str(), record.size());
  return targetIndex;
}
}