#include "options.h"

namespace MisakiDB{
Options::Options() {

}

size_t Options::getIndexBufferPoolSize() const { return 100; }

size_t Options::getDataBufferPoolSize() const { return 100; }
}
