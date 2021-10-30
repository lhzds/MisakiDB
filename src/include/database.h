#pragma once
#include "globals.h"
#include "options.h"
#include "b_plus_tree.h"
#include "data_accessor.h"
#include "buffer_pool_manager.h"
#include "file_manager/index_file_manager.h"
#include "file_manager/data_file_manager.h"

namespace MisakiDB {
    class DataBase
    {
    public:
        DataBase(const NameType &nameType, const Options &options);
        ValueType get(const KeyType &key) const;
        bool remove(const KeyType &key);
        bool exist(const KeyType &key) const;
        void set(const KeyType &key, const ValueType &value);

    protected:
        RecordIDType getRecordID_helper(const KeyType &key) const;
        ValueType getValue_helper(RecordIDType recordID) const;

        RecordIDType removeRecordID_helper(const KeyType &key);
        bool removeRecord_helper(RecordIDType recordID);

        RecordIDType addRecord_helper(const KeyType &key, const ValueType &value);
        void addRecordID_helper(RecordIDType recordID);

    private:
        BPlusTree m_bPlusTree;
        DataAccessor m_dataAccessor;
        BufferPoolManager m_indexBufferPoolManager, m_dataBufferPoolManager;
        IndexFileManager m_indexFileManager;
        DataFileManager m_dataFileManager;

        std::shared_mutex m_readWriteLock;
    };
}
