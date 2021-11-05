#include "server.h"

namespace MisakiDB{
Server::Server() {
  // Initialize DLL
  WSADATA wsdata;
  if (0 != WSAStartup(MAKEWORD(2, 2), &wsdata)) throw "WSAStartup Error";

  // Create socket
  this->m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == this->m_serverSocket) throw "Create Socket Error";

  // Bind socket
  sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_port = htons(SERVER_PORT);
  sockAddr.sin_addr.S_un.S_addr = INADDR_ANY;
  if (SOCKET_ERROR == bind(this->m_serverSocket, (sockaddr*)&sockAddr, sizeof(sockAddr))) {
    throw "Bind Error";
  }

  // Begin listening
  if (SOCKET_ERROR == listen(this->m_serverSocket, 10)) throw "Listen Error";
}

Server::~Server() {
  // Close socket
  closesocket(this->m_serverSocket);

  // WSACleanup
  WSACleanup();
}

auto Server::open(const std::string &databaseName, const Options &options) {
  // Lock
  this->m_databasesMutex.lock();

  // Check if the database is already opened
  auto database { std::find_if(begin(this->m_databases), end(this->m_databases),
                         [&](const DataBase &database) {
                           return database.getName() == databaseName; }) };

  // If the database does not opened, open it
  if (database != end(this->m_databases)) {
    this->m_databases.emplace_back(databaseName, options);
    database = --end(this->m_databases);
  }

  // Increase the inuse count
  database->use();

  // Unlock
  this->m_databasesMutex.unlock();

  return database;
}

void Server::close(std::_List_iterator<DataBase> &database) {
  // Lock
  this->m_databasesMutex.lock();

  // Decrease the inuse count
  database->unuse();

  // If this is the last user that closes the database, then remove it
  if (not database->inuse()) this->m_databases.erase(database);

  // Unlock
  this->m_databasesMutex.unlock();
}

void Server::start() {
  while (true) {
    // Accept client
    SOCKET clientSocket { accept(this->m_serverSocket, nullptr, nullptr) };
    if (INVALID_SOCKET == clientSocket) throw "Accept Error";

    // Start serving
    std::thread { &Server::serve_helper, this, clientSocket }.detach();
  }
}

void Server::serve_helper(SOCKET clientSocket) {
  std::_List_iterator<DataBase> database;

  while (true) {
    // Message buffer
    char buffer[4096] { 0 };
    // Get message total length
    recv(clientSocket, buffer, sizeof (buffer) - 1, 0);
    int64_t length { atoll(buffer) };
    memset(buffer, 0, sizeof(buffer));

    // Receive message until length equals or belows zero
    std::string message;
    while ((length -= recv(clientSocket, buffer, sizeof (buffer) - 1, 0)) > 0) {
      // Receive the data and append it to message
      message += buffer;
      memset(buffer, 0, sizeof(buffer));
    }

    // Find the first space
    uint64_t index { 0 };
    while (index + 1 < message.length() and message.at(++index) != ' ');

    // Slice out the operation
    std::string operation { message.substr(0, index++) };


    // Switch on the operation
    if ("OPEN" == operation) {
      // Slice out the database name
      std::string databaseName { message.substr(index) };

      // Perform open operation
      database = open(databaseName, Options { });

      // Reply message
      send(clientSocket, "OPEN SUCCESSFUL", 15, 0);
    }

    else if ("GET" == operation) {
      // Slice out the key
      KeyType key { message.substr(index) };

      // Perform get operation
      if (database->get(key, message)) message = "GET FAILED: KEY DOES NOT EXIST";

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    else if ("SET" == operation) {
      // Find the next space
      uint64_t nextIndex { index };
      while (nextIndex < message.length() - 1 and message.at(++nextIndex) != ' ');

      // Slice out the key
      KeyType key { message.substr(index, nextIndex++ - index) };

      // Slice out the value
      ValueType value { message.substr(nextIndex) };

      // Perform set operation
      database->set(key, value);

      // Reply message
      send(clientSocket, "SET SUCCESSFUL", 14, 0);
    }

    else if ("REMOVE" == operation) {
      // Slice out the key
      KeyType key { message.substr(index) };

      // Perform remove operation
      if (database->remove(key)) message = "REMOVE SUCCESSFUL";
      else message = "REMOVE FAILED: KEY DOES NOT EXIST";

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    else if ("EXIST" == operation) {
      // Slice out the key
      KeyType key { message.substr(index) };

      // Perform exist operation
      if (database->exist(key)) message = "EXIST";
      else message = "NOT EXIST";

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    else if ("CLOSE" == operation) {
      // Close database
      close(database);

      // Reply message
      send(clientSocket, "DATABASE CLOSED", 15, 0);

      // Close socket
      closesocket(clientSocket);
      break;
    }
  }
}
}
