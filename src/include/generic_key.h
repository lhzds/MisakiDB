#pragma once
#include "globals.h"

namespace MisakiDB {
template<size_t KeySize>
class GenericKey {
public:
  GenericKey() = default;
  GenericKey(const std::string &data) {
    strcpy_s(m_data, KeySize, data.c_str());
  }
  GenericKey(const char *data) {
    strcpy_s(m_data, KeySize, data);
  };
  const char *getData() const { return m_data; };
  
private:
  char m_data[KeySize]{};
};

template<size_t KeySize>
class GenericComparator {
public:
  inline int operator()(const GenericKey<KeySize> &lhs, const GenericKey<KeySize> &rhs) const {
    const char *lhsData = lhs.getData();
    const char *rhsData = rhs.getData();
    for (size_t i = 0; i < KeySize; ++i) {
      if (lhsData[i] != rhsData[i]) {
        return lhsData[i] - rhsData[i];
      }
    }
    return 0;
  }
};
}
