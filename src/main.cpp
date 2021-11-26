#include "server.h"

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

  // Start the server to serve
  // When the quit event triggers, the deconstructor will do the cleaning job
  StartServer();

  // Signal clean up finish event
  SetEvent(FINISH_EVENT);
  return 0;
}
