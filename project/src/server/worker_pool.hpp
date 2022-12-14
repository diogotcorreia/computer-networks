#ifndef WORKER_POOL_H
#define WORKER_POOL_H

#include <mutex>
#include <thread>

#include "common/constants.hpp"

class WorkerPool {
  // TODO change type to worker array
  std::thread pool[TCP_WORKER_POOL_SIZE];
  bool busy_threads[TCP_WORKER_POOL_SIZE];
  std::mutex busy_threads_locks;

 public:
  WorkerPool();
};

// TODO create worker class

#endif
