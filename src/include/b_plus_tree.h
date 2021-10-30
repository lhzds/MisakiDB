#pragma once
#include "globals.h"

namespace MisakiDB {
    class BPlusTree {
    public:
        BPlusTree();
        void insert();
        bool remove();
        bool exist();
        ValueType get();
    };
}
