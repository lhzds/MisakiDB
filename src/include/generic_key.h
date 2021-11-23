#include "globals.h"

template <size_t KeySize>
class GenericKey {
public:
  GenericKey(const std::string &data) {
    memcpy(m_data, data.c_str(), KeySize);
  }
  GenericKey(const char *data) {
    memcpy(m_data, data, KeySize);
  };
  const char *getData() const { return m_data; };
private:
  char m_data[KeySize];
};

template <size_t KeySize>
class GenericComparator {
public:
  inline int operator () (const GenericKey<KeySize> &lhs, const GenericKey<KeySize> &rhs) const {
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
