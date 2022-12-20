#ifndef WORKER_POOL_H
#define WORKER_POOL_H

#include <condition_variable>
#include <mutex>
#include <thread>

#include "common/constants.hpp"
#include "server_state.hpp"

class WorkerPool;

class Worker {
  std::thread thread;

  void execute();

 public:
  int tcp_socket_fd = -1;
  bool shutdown = false;
  bool to_execute = false;
  WorkerPool *pool;
  uint32_t worker_id = 0;
  std::mutex lock;
  std::condition_variable cond;

  Worker();
  ~Worker();
};

class WorkerPool {
  GameServerState &server_state;
  Worker workers[TCP_WORKER_POOL_SIZE];
  bool busy_threads[TCP_WORKER_POOL_SIZE];
  std::mutex busy_threads_lock;

 public:
  WorkerPool(GameServerState &__server_state);
  void delegateConnection(int connection_fd);
  void freeWorker(uint32_t worker_id);
};

std::string read_packet_id(int fd);

class NoWorkersAvailableException : public std::runtime_error {
 public:
  NoWorkersAvailableException()
      : std::runtime_error(
            "All TCP workers are busy, cannot handle the incoming "
            "connection.") {}
};

#endif
