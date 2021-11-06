#include "globals.h"

namespace MisakiDB {
class ThreadPool final
{
public:
  ThreadPool(size_t poolSize);
  ~ThreadPool();

  // Delete the ctor and copy assignment operator
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool &operator=(const ThreadPool&) = delete;

  // Submit a task to the thread pool
  void submit(auto function, auto... args);
private:
  // Worker threads
  std::vector<std::thread> m_workers;

  // Task queue
  std::queue<std::function<void()>> m_tasks;
  // Mutex for task queue
  std::mutex m_tasksLock;
  // Condition variable for task queue
  std::condition_variable m_tasksCondVar;

  // Stop sign
  bool m_stop { false };
};

void ThreadPool::submit(auto function, auto... args) {
  // Lock the task queue
  std::unique_lock<std::mutex> tasksLock { this->m_tasksLock };

  // Construct task and put it into the task queue
  this->m_tasks.emplace(std::bind(std::forward<decltype (function)>(function),
                                  std::forward<decltype (args)>(args)...));

  // Wake up a worker to do the task
  this->m_tasksCondVar.notify_one();
}
}
