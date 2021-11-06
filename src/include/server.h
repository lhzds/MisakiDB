#pragma once
#include "globals.h"
#include "options.h"
#include "database.h"
#include "thread_pool.h"
#include "winsock2.h"
#include <list>

namespace MisakiDB {
class Server {
public:
  Server();
  ~Server();
  auto open(const std::string &databaseName, const Options &options);
  void close(std::_List_iterator<DataBase> &database);

  void start();

protected:
  void serve_helper(SOCKET clientSocket);

private:
  SOCKET m_serverSocket;

  std::mutex m_databasesMutex;
  std::list<DataBase> m_databases;

  ThreadPool m_threadPool;
};
}
