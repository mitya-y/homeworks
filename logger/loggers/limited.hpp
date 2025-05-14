#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class LimitedLogger : public Logger {
private:
  class LimitedQueue {
  private:
    std::queue<Message> queue;
    std::mutex mutex;
    std::atomic_bool done = false;
    std::size_t size = 0;

  public:
    LimitedQueue(std::size_t size) : size(size) {}

    void push(Message &&msg) {
      std::lock_guard lock(mutex);
      queue.push(std::move(msg));
    }

    bool pop(Message &msg) {
      while (!done) {
        std::unique_lock lock(mutex);
        if (queue.empty()) {
          lock.unlock();
          std::this_thread::sleep_for(10ns);
        } else {
          msg = std::move(queue.front());
          queue.pop();
          break;
        }
      }
      return !done;
    }

    ~LimitedQueue() {
      done = true;
    }
  };
  LimitedQueue queue;

public:
  LimitedLogger(std::ostream &out, std::size_t size = 100) : Logger(out), queue(size) {
    log_thread = std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        Message msg;
        if (queue.pop(msg)) {
          std::lock_guard lock(log_mutex);
          out << msg.data;
        }
      }
    });
  }

  void log(std::string_view msg) override {
    queue.push(Message(msg));
  }
};

