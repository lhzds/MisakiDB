#include "database.h"

namespace MisakiDB{
    NameType DataBase::getName() const { return this->m_databaseName; }

    uint64_t DataBase::inuse() const { return this->m_inuse; }

    void DataBase::use() { ++this->m_inuse; }

    void DataBase::unuse() { --this->m_inuse; }
}
