#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class CondVarUnlimitedLogger : public Logger {
private:
  std::queue<Message> queue;
  std::mutex mutex;
  std::condition_variable cv;
  std::atomic_bool end = false;

public:
  CondVarUnlimitedLogger(std::ostream &out) : Logger(out) {
    log_thread = std::jthread([&](std::stop_token stop_token) {
      while (!end && !stop_token.stop_requested()) {
        Message msg;
        bool was_poped = false;
        std::unique_lock lock(mutex);
        cv.wait(lock, [this, &lock]() {
          return !queue.empty() || end;
        });

        if (!queue.empty()) {
          msg = std::move(queue.front());
          queue.pop();
          was_poped = true;
        }
        lock.unlock();

        if (was_poped) {
          out << msg.data;
        } else {
          std::this_thread::sleep_for(10ns);
        }
      }
    });
  }

  void log(std::string_view msg) override {
    if (end) {
      return;
    }
    std::lock_guard lock(mutex);
    queue.push(Message(msg));
    cv.notify_one();
  }

  ~CondVarUnlimitedLogger() {
    cv.notify_one();
    end = true;
    log_thread.join();
  }
};
