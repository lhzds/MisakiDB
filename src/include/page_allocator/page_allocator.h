#pragma once
#include "globals.h"

namespace MisakiDB {
    class PageAllocator {
    public:
        PageAllocator();
        virtual PageIDType allocate() = 0;
        virtual void deallocate(PageIDType pageID) = 0;
    };
}
