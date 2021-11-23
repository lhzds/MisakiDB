#pragma once
#include <bits/stdc++.h>
#include "generic_key.h"

namespace MisakiDB {
// using KeyType = GenericKey<24>;
// using ValueType = std::string;
using RecordIDType = uint64_t;
using ByteType = char;
using FileSizeType = uint64_t;
using RecordSizeType = int16_t;
using PageIDType = uint32_t;
using FrameIDType = uint32_t;

constexpr RecordIDType INVALID_RECORD_ID { std::numeric_limits<uint64_t>::max() };
constexpr PageIDType INVALID_PAGE_ID { std::numeric_limits<uint32_t>::max() };

constexpr uint32_t PAGE_SIZE { 4096 };

constexpr uint16_t SERVER_PORT { 8888 };

enum class FileType {INVALID, INDEX, DATA, FREE_SPACE_MAP};
}
