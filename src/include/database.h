#pragma once
#include "globals.h"
#include "options.h"
#include "b_plus_tree.h"
#include "data_accessor.h"
#include "buffer/buffer_pool_manager.h"
#include "file_store/index_file_store.h"
#include "file_store/data_file_store.h"

namespace MisakiDB {
    class DataBase
    {
    public:
        DataBase() = default;
        DataBase(const NameType &nameType, const Options &options);
        ValueType get(const KeyType &key) const;
        bool remove(const KeyType &key);
        bool exist(const KeyType &key) const;
        void set(const KeyType &key, const ValueType &value);

        void use();
        void unuse();
        uint64_t inuse() const;

        NameType getName() const;

      protected:
        RecordIDType getRecordID_helper(const KeyType &key) const;
        ValueType getValue_helper(RecordIDType recordID) const;

        RecordIDType removeRecordID_helper(const KeyType &key);
        bool removeRecord_helper(RecordIDType recordID);

        RecordIDType addRecord_helper(const KeyType &key, const ValueType &value);
        void addRecordID_helper(RecordIDType recordID);

    private:
        std::atomic_uint64_t m_inuse { 0 };
        NameType m_databaseName;
        BPlusTree m_bPlusTree;
        DataAccessor m_dataAccessor;
        BufferPoolManager m_indexBufferPoolManager, m_dataBufferPoolManager;
        IndexFileStore m_indexFileManager;
        DataFileStore m_dataFileManager;

        std::shared_mutex m_readWriteLock;
    };
}
