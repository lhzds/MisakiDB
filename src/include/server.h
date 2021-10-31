#pragma once
#include "globals.h"
#include "options.h"
#include "database.h"
#include "winsock2.h"
#include <vector>

namespace MisakiDB {
    class Server {
    public:
        Server();
        ~Server();
        void create(const NameType &databaseName, const Options &options);
        void close(const NameType &databaseName);

        ValueType get(const KeyType &key) const;
        ValueType get(const KeyType &key1, const KeyType &key2) const;
        void set(const KeyType &key, const ValueType &value);
        bool exist(const KeyType &key) const;
        bool remove(const KeyType &key);

        void start();

    protected:
        void serve_helper(SOCKET clientSocket);

    private:
        SOCKET m_serverSocket;

        std::mutex m_databasesMutex;
        std::vector<DataBase> m_databases;
    };
}
