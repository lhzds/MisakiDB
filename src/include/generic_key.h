#pragma once
#include "globals.h"

namespace MisakiDB {
template<size_t KeySize>
class GenericKey {
public:
  GenericKey() = default;
  GenericKey(const std::string &data) {
    assert(data.size() <= KeySize);
    memcpy(m_data, data.c_str(), data.size());
  }
  
  void setData(const std::string &data) {
    assert(data.size() <= KeySize);
    memcpy(m_data, data.c_str(), data.size());
    memset(m_data + data.size(), 0, KeySize - data.size());
  }
  
  const char *getData() const { return m_data; };
  std::string toStr() const { return std::string(m_data, KeySize); }
  
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
