#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class LimitedLogger : public Logger {
private:
  std::queue<Message> queue;
  std::mutex mutex;
  std::atomic_bool end = false;
  std::size_t capacity = 0;

public:
  LimitedLogger(std::ostream &out, std::size_t size = 100) : Logger(out), capacity(size) {
    log_thread = std::jthread([&](std::stop_token stop_token) {
      while (!end && !stop_token.stop_requested()) {
        Message msg;
        while (!end) {
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

        out << msg.data;
      }
    });
  }

  void log(std::string_view msg) override {
    while (!end) {
      std::unique_lock lock(mutex);
      if (end) {
        break;
      }

      if (queue.size() < capacity) {
        queue.push(Message(msg));
        break;
      } else {
        lock.unlock();
        std::this_thread::sleep_for(10ns);
      }
    }
  }

  ~LimitedLogger() {
    end = true;
    log_thread.join();
  }
};

