#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class UnlimitedLogger : public Logger {
  std::queue<Message> queue;
  std::mutex mutex;
  std::atomic_bool end = false;

public:
  UnlimitedLogger(std::ostream &out) : Logger(out) {
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
        if (end) {
          break;
        }

        out << msg.data;
      }
    });
  }

  void log(std::string_view msg) override {
    if (end) {
      return;
    }
    std::lock_guard lock(mutex);
    queue.push(Message(msg));
  }

  ~UnlimitedLogger() {
    end = true;
    log_thread.join();
  }
};

