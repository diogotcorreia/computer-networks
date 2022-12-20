#include "worker_pool.hpp"

#include <unistd.h>

#include <iostream>

#include "common/protocol.hpp"

Worker::Worker() {
  thread = std::thread(&Worker::execute, this);
}

Worker::~Worker() {
  lock.lock();
  shutdown = true;
  lock.unlock();
  thread.join();
}

void Worker::execute() {
  while (!shutdown) {
    std::unique_lock<std::mutex> unique_lock(lock);
    try {
      while (!to_execute && !shutdown) {
        cond.wait(unique_lock);
      }

      if (shutdown) {
        return;
      }

      std::string packet_id = read_packet_id(tcp_socket_fd);

      std::cout << "Got packet ID " << packet_id << std::endl;

      pool->server_state.callTcpPacketHandler(packet_id, tcp_socket_fd);

      close(tcp_socket_fd);

    } catch (std::exception &e) {
      std::cerr << "Worker encountered an exception while running: " << e.what()
                << std::endl;
    } catch (...) {
      std::cerr << "Worker encountered an unknown exception while running."
                << std::endl;
    }
    to_execute = false;
    pool->freeWorker(worker_id);
  }
}

WorkerPool::WorkerPool(GameServerState &__server_state)
    : server_state{__server_state} {
  for (uint32_t i = 0; i < TCP_WORKER_POOL_SIZE; ++i) {
    busy_threads[i] = false;
    workers[i].pool = this;
    workers[i].worker_id = i;
  }
}

void WorkerPool::delegateConnection(int connection_fd) {
  std::scoped_lock<std::mutex> slock(busy_threads_lock);

  for (size_t i = 0; i < TCP_WORKER_POOL_SIZE; ++i) {
    if (!busy_threads[i]) {
      // Found an available worker!
      std::scoped_lock<std::mutex> worker_lock(workers[i].lock);

      busy_threads[i] = true;
      workers[i].tcp_socket_fd = connection_fd;
      workers[i].to_execute = true;

      workers[i].cond.notify_one();

      server_state.cdebug << "Delegated to worker #" << i << std::endl;
      return;
    }
  }

  throw NoWorkersAvailableException();
}

void WorkerPool::freeWorker(uint32_t worker_id) {
  std::scoped_lock<std::mutex> slock(busy_threads_lock);
  if (worker_id < TCP_WORKER_POOL_SIZE) {
    busy_threads[worker_id] = false;
  }
}

std::string read_packet_id(int fd) {
  char id[PACKET_ID_LEN + 1];
  size_t to_read = PACKET_ID_LEN;

  while (to_read > 0) {
    ssize_t n = read(fd, &id[PACKET_ID_LEN - to_read], to_read);
    if (n < 0) {
      std::cerr << "Received malformated packet ID" << std::endl;
      throw new InvalidPacketException();
    }
    to_read -= (size_t)n;
  }

  id[PACKET_ID_LEN] = '\0';
  return std::string(id);
}
