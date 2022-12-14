#include "worker_pool.hpp"

WorkerPool::WorkerPool() {
  for (size_t i = 0; i < TCP_WORKER_POOL_SIZE; ++i) {
    busy_threads[i] = false;
  }
}
