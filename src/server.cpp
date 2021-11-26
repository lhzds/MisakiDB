#include "server.h"

namespace MisakiDB{
Server::Server() : m_threadPool { 1000 } {
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
  std::unique_lock<std::mutex> lock { this->m_databasesMutex };

  // Check if the database is already opened
  auto database { std::find_if(begin(this->m_databases), end(this->m_databases),
                         [&](const DataBase &database) {
                           return database.getName() == databaseName; }) };

  // If the database does not opened, open it
  if (database == end(this->m_databases)) {
    this->m_databases.emplace_back(databaseName, options);
    database = --end(this->m_databases);
  }

  // Increase the inuse count
  database->use();

  return database;
}

void Server::close(std::_List_iterator<DataBase> &database) {
  // Lock
  std::unique_lock<std::mutex> lock { this->m_databasesMutex };

  // Decrease the inuse count
  database->unuse();

  // If this is the last user that closes the database, then remove it
  if (not database->inuse()) this->m_databases.erase(database);
}

void Server::start() {
  while (true) {
    // Accept client
    SOCKET clientSocket { accept(this->m_serverSocket, nullptr, nullptr) };
    if (INVALID_SOCKET == clientSocket) std::cout << "Accept Error" << std::endl;

    // Start serving
    else this->m_threadPool.submit(&Server::serve, this, clientSocket);
  }
}

void Server::serve(SOCKET clientSocket) {
  std::_List_iterator<DataBase> database;
  bool notOpen { true };

  // Message buffer
  char buffer[8192] { 0 };

  while (true) {
    // Get message total length
    recv(clientSocket, buffer, sizeof (buffer) - 1, 0);
    int64_t length { atoll(buffer) };
    memset(buffer, 0, sizeof(buffer));

    // Receive message until length equals or belows zero
    std::string message;
    do {
      // Receive the data and check whether the socket is closed
      int ret { recv(clientSocket, buffer, sizeof (buffer) - 1, 0) };
      if (SOCKET_ERROR == ret or 0 == ret) { closesocket(clientSocket); return; }
      else length -= ret;

      // append it to message
      message += buffer;
      memset(buffer, 0, sizeof(buffer));
    } while (length > 0);

    // Find the first space
    uint64_t index { 0 };
    while (index + 1 < message.length() and message.at(++index) != ' ');

    // Slice out the operation
    std::string operation { message.substr(0, index++) };

    // Make all the operation uppercase
    for (auto &ch : operation) ch = toupper(ch);

    // Check if the database is opened
    if (notOpen) {
      if ("OPEN" == operation) {
        // The user opens a database
        notOpen = false;

        // Slice out the database name
        std::string databaseName { message.substr(index) };

        // Perform open operation
        database = open(databaseName, Options { });

        // Reply message
        send(clientSocket, "OPEN SUCCESSFUL\n", 16, 0);
      }

      // The user have not opened any database yet, reply error message
      else send(clientSocket, "NO DATABASE OPENED YET\n", 23, 0);
    }

    // If the database is opened, then switch on the operation
    else if ("OPEN" == operation) {
      // Close the current database
      close(database);

      // Slice out the database name
      std::string databaseName { message.substr(index) };

      // Perform open operation
      database = open(databaseName, Options { });

      send(clientSocket, "OPEN SUCCESSFUL\n", 16, 0);
    }

    else if ("GET" == operation) {
      // Slice out the key
      std::string key { message.substr(index) };

      // Perform get operation
      if (database->get(key, message)) message = "GET FAILED: KEY DOES NOT EXIST\n";

      // Reply length
      std::string messageLength { std::to_string(message.length()) };
      send(clientSocket, messageLength.c_str(), messageLength.length(), 0);

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    else if ("SET" == operation) {
      // Find the next space
      uint64_t nextIndex { index };
      while (nextIndex < message.length() - 1 and message.at(++nextIndex) != ' ');

      // Slice out the key
      std::string key { message.substr(index, nextIndex++ - index) };

      // Slice out the value
      std::string value { message.substr(nextIndex) };

      // Perform set operation
      database->set(key, value);

      // Reply message
      send(clientSocket, "SET SUCCESSFUL\n", 15, 0);
    }

    else if ("DELETE" == operation) {
      // Slice out the key
      std::string key { message.substr(index) };

      // Perform delete operation
      if (database->remove(key)) message = "DELETE SUCCESSFUL\n";
      else message = "DELETE FAILED: KEY DOES NOT EXIST\n";

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    else if ("EXIST" == operation) {
      // Slice out the key
      std::string key { message.substr(index) };

      // Perform exist operation
      if (database->exist(key)) message = "EXIST\n";
      else message = "NOT EXIST\n";

      // Reply message
      send(clientSocket, message.c_str(), message.length(), 0);
    }

    // Unrecognized command
    else send(clientSocket, "UNRECOGNIZED COMMAND\n", 21, 0);
  }
}
}
