#include "globals.h"

namespace MisakiDB {
class IndexFileFreePage {
private:
  static constexpr uint32_t MAGIC_NUM = 18827755;
  
  uint32_t m_magic;
  PageIDType m_nextFreePageID;
public:
  void init();
  
  bool isFreePage() const;
  PageIDType getNextFreePageID() const;
  void setNextFreePageID(PageIDType nextFreePageID);
};
}

