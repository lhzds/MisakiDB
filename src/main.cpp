#include "server.h"

namespace MisakiDB {
uint32_t SERVER_PORT;
size_t WORKER_THREADS_NUMBER;
size_t INDEX_BUFFER_POOL_SIZE;
size_t DATA_BUFFER_POOL_SIZE;
}

// Clean up finish event
HANDLE FINISH_EVENT { CreateEvent(NULL, TRUE, FALSE, NULL) };
// Server socket
SOCKET SERVER_SOCKET;
// CtrlHandler Callback function
BOOL WINAPI ConsoleHandler(DWORD CEvent) {
  switch (CEvent) {
  case CTRL_C_EVENT: [[fallthrough]];
  case CTRL_BREAK_EVENT: [[fallthrough]];
  case CTRL_CLOSE_EVENT: [[fallthrough]];
  case CTRL_LOGOFF_EVENT: [[fallthrough]];
  case CTRL_SHUTDOWN_EVENT: [[fallthrough]];
  default:
    // Close server socket
    closesocket(SERVER_SOCKET);

    // Wait for cleaning job to finish
    WaitForSingleObject(FINISH_EVENT, INFINITE);

    // Do the final clean up and exits the process
    CloseHandle(FINISH_EVENT);
    ExitProcess(0);
  }
  return TRUE;
}

void StartServer() {
  MisakiDB::Server server;
  SERVER_SOCKET = server.getServerSocket();
  server.start();
}

int main() {
  // Setup quit callback function
  SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);

  // Read database options from file
  if (not std::ifstream { "MisakiDB.ini" }.is_open()) {
    // Create a new option file
    WritePrivateProfileString("Options", "server-port", "6666", ".\\MisakiDB.ini");
    WritePrivateProfileString("Options", "worker-threads-number",
      std::to_string(std::thread::hardware_concurrency()).c_str(), ".\\MisakiDB.ini");
    WritePrivateProfileString("Options", "index-buffer-pool-size", "100", ".\\MisakiDB.ini");
    WritePrivateProfileString("Options", "data-buffer-pool-size", "100", ".\\MisakiDB.ini");
  }
  MisakiDB::SERVER_PORT = GetPrivateProfileInt("Options", "server-port", 6666, ".\\MisakiDB.ini");
  MisakiDB::WORKER_THREADS_NUMBER = GetPrivateProfileInt("Options", "worker-threads-number",
      std::thread::hardware_concurrency(), ".\\MisakiDB.ini");
  MisakiDB::INDEX_BUFFER_POOL_SIZE =
      GetPrivateProfileInt("Options", "index-buffer-pool-size", 100, ".\\MisakiDB.ini");
  MisakiDB::DATA_BUFFER_POOL_SIZE =
      GetPrivateProfileInt("Options", "data-buffer-pool-size", 100, ".\\MisakiDB.ini");

  // Print Banner Information
  std::string banner {
R"(Misaki DataBase - Super fast key-value storage database with enhanced Mikoto Protocal

[Database Options]
ServerPort: )" + std::to_string(MisakiDB::SERVER_PORT) + R"(
WorkerThreadsNumber: )" + std::to_string(MisakiDB::WORKER_THREADS_NUMBER) + R"(
IndexBufferPoolSize: )" + std::to_string(MisakiDB::INDEX_BUFFER_POOL_SIZE) + R"(
DataBufferPoolSize: )" + std::to_string(MisakiDB::DATA_BUFFER_POOL_SIZE) + R"(

Server has been successfully started.

Powered by Misaki Team, SCNU. Copyright 2021, All Rights Reserved.)" };
  std::cout << banner;

  // Start the server to serve
  // When the quit event triggers, the deconstructor will do the cleaning job
  StartServer();

  // Signal clean up finish event
  SetEvent(FINISH_EVENT);
  return 0;
}
