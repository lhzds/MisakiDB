#pragma once
#include <bits/stdc++.h>

namespace MisakiDB {
using KeyType = std::string;
using ValueType = std::string;
using RecordIDType = uint64_t;
using ByteType = char;
using PageIDType = uint64_t;
using FrameIDType = uint32_t;

constexpr RecordIDType INVALID_RECORD_ID { std::numeric_limits<uint64_t>::max() };
constexpr PageIDType INVALID_PAGE_ID { std::numeric_limits<uint64_t>::max() };

constexpr size_t PAGE_SIZE { 4096 };

constexpr uint16_t SERVER_PORT { 8888 };

enum class FILE_TYPE {INDEX, DATA, FREE_SPACE_MAP};
}
