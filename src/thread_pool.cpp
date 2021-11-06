#include "thread_pool.h"

namespace MisakiDB {
ThreadPool::ThreadPool(size_t poolSize) {
  for (size_t workerCount { 0 }; workerCount < poolSize; ++workerCount) {
    this->m_workers.emplace_back([&] {
      while (true) {
        // Lock the task queue
        std::unique_lock<std::mutex> tasksLock { this->m_tasksLock };

        // Wait for the task or the end sign signals
        this->m_tasksCondVar.wait(tasksLock, [&] {
          return this->m_stop or not this->m_tasks.empty();
        });

        // If the end sign signals, returns when the task queue is empty
        if (this->m_stop and this->m_tasks.empty()) return;

        // Fetch a task and store it to the task
        std::function<void()> task { std::move(this->m_tasks.front()) };
        this->m_tasks.pop();

        // Release the lock
        tasksLock.unlock();

        // Do the task
        task();
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  // Lock the stop sign
  this->m_tasksLock.lock();

  // Set the stop sign to true
  this->m_stop = true;

  // Release the lock
  this->m_tasksLock.unlock();

  // Wake up all the workers to stop
  this->m_tasksCondVar.notify_all();

  // Wait for all the workers to stop
  for (auto &worker : this->m_workers) worker.join();
}
}
