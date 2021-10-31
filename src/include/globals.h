#pragma once
#include <bits/stdc++.h>

namespace MisakiDB {
using NameType = std::string;
using KeyType = std::string;
using ValueType = std::string;
using RecordIDType = uint64_t;
using PageIDType = uint64_t;
using RawPage = uint8_t*;

constexpr RecordIDType INVALID_RECORD_ID { std::numeric_limits<uint64_t>::max() };
constexpr PageIDType INVALID_PAGE_ID { std::numeric_limits<uint64_t>::max() };

size_t PAGE_SIZE { 4096 };

uint16_t SERVER_PORT { 8888 };
}
