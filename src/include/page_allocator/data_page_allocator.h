#pragma once
#include "globals.h"
#include "page_allocator.h"

namespace MisakiDB {
    class DataPageAllocator final : public PageAllocator {
    public:
        DataPageAllocator();
        virtual PageIDType allocate() override;
        virtual void deallocate(PageIDType pageID) override;
    };
}

