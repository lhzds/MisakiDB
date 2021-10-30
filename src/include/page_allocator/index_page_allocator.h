#pragma once
#include "globals.h"
#include "page_allocator.h"

namespace MisakiDB {
    class IndexPageAllocator final : public PageAllocator {
    public:
        IndexPageAllocator();
        virtual PageIDType allocate() override;
        virtual void deallocate(PageIDType pageID) override;
    };
}
