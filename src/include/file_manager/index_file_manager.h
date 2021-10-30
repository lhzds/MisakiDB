#pragma once
#include "globals.h"
#include "file_manager.h"

namespace MisakiDB {
class IndexFileManager : public FileManager {
public:
  IndexFileManager();
  virtual RawPage getRawPage(PageIDType pageID) const override;
  virtual PageIDType allocatePage() override;
  virtual void deallocatePage(PageIDType pageID) override;
};
}
