#pragma once
#include <bits/stdc++.h>
#include "generic_key.h"

namespace MisakiDB {
using ByteType = char;
using FileSizeType = uint64_t;
using RecordSizeType = int16_t;
using PageIDType = uint32_t;
using FrameIDType = uint32_t;
using BlobIDType = uint64_t;
union RecordIDType {
  uint64_t recordID;
  struct {
    int reserved: 16;
    PageIDType pageID: 32;
    int slotArrayIndex: 16;
  };
};

constexpr RecordIDType INVALID_RECORD_ID { std::numeric_limits<RecordIDType>::max() };
constexpr PageIDType INVALID_PAGE_ID { std::numeric_limits<PageIDType>::max() };
constexpr BlobIDType INVALID_BLOB_ID {std::numeric_limits<BlobIDType>::max()};

constexpr size_t RECORD_KEY_SIZE { 24 };

constexpr size_t PAGE_SIZE { 4096 };

extern uint32_t SERVER_PORT;

extern size_t WORKER_THREADS_NUMBER;

extern size_t INDEX_BUFFER_POOL_SIZE;

extern size_t DATA_BUFFER_POOL_SIZE;

enum class FileType {INVALID, INDEX, DATA, FREE_SPACE_MAP, BLOB};
}
