#pragma once
#include "globals.h"

namespace MisakiDB {
class DataAccessor
{
public:
  DataAccessor();
  ValueType get(RecordIDType recordID) const;
  bool remove(RecordIDType recordID);
  RecordIDType add(const KeyType &key, const ValueType &value);
};
}

