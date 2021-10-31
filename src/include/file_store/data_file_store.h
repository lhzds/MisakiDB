#pragma once
#include "globals.h"
#include "file_store.h"

namespace MisakiDB {
class DataFileStore final : public FileStore {
public:
  DataFileStore();
};
}

