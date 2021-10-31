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

    void Server::create(const NameType &databaseName, const Options &options) {
      // Lock
      this->m_databasesMutex.lock();

      // Check if the database is already created
      auto iter { std::find_if(begin(this->m_databases), end(this->m_databases),
                   [&](const DataBase &database) {
                     return database.getName() == databaseName; }) };

      // If the database does not exist, create one
      if (iter != end(this->m_databases)) this->m_databases.emplace_back(databaseName, options);

      // Increase the inuse count
      iter->use();

      // Unlock
      this->m_databasesMutex.unlock();
    }

    void Server::close(const NameType &databaseName) {
      // Lock
      this->m_databasesMutex.lock();

      // Check if the database exists
      auto iter { std::find_if(begin(this->m_databases), end(this->m_databases),
                             [&](const DataBase &database) {
                               return database.getName() == databaseName; }) };

      if (iter != end(this->m_databases)) {
        // Decrease the inuse count
        iter->unuse();

        // If this is the last user that closes the database, then remove it
        if (not iter->inuse()) this->m_databases.erase(iter);
      }

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
      // Receive message until "end" received
      std::string message;
      char buffer[4096] { 0 };
      while (recv(clientSocket, buffer, sizeof (buffer) - 1, 0)) {
        if (not strcmp(buffer, "end")) break;
        message += buffer;
        memset(buffer, 0, sizeof(buffer));
      }

      // Find the first space
      uint64_t index { 0 };
      while (index < message.length() - 1 and message.at(++index) != ' ');

      // Slice out the operation
      std::string operation { message.substr(0, index) };

      // Perform the operation
      if ("create" == operation) {

      } else if ("get" == operation) {

      } else if ("set" == operation) {

      } else if ("remove" == operation) {

      } else if ("exist" == operation) {

      }

      // Reply message
      message = "Long live MisakiDB!";
      send(clientSocket, message.c_str(), message.length(), 0);

      // Close socket
      closesocket(clientSocket);
    }
}
