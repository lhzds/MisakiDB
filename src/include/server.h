#pragma once
#include "globals.h"
#include "options.h"
#include "database.h"

namespace MisakiDB {
    class Server {
    public:
        Server();
        void create(const NameType &databaseName, const Option &option);
        void close(const NameType &databaseName);
        ValueType get(const KeyType &key) const;
        ValueType get(const KeyType &key1, const KeyType &key2) const;
        void set(const KeyType &key, const ValueType &value);
        bool exist(const KeyType &key) const;
        bool remove(const KeyType &key);

    private:
        DataBase m_dataBase;
    };
}
