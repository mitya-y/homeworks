#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class CondVarLimitedLogger : public Logger {
private:
  std::queue<Message> queue;
  std::mutex mutex;
  std::condition_variable cv_pop;
  std::condition_variable cv_push;
  std::size_t capacity = 0;
  std::atomic_bool end = false;

public:
  CondVarLimitedLogger(std::ostream &out, std::size_t size = 10) : Logger(out), capacity(size) {
    log_thread = std::jthread([&](std::stop_token stop_token) {
      while (!end && !stop_token.stop_requested()) {
        Message msg;
        bool was_poped = false;
        std::unique_lock lock(mutex);
        cv_pop.wait(lock, [this]() {
          return !queue.empty() || end;
        });

        if (!queue.empty()) {
          msg = std::move(queue.front());
          queue.pop();
          cv_push.notify_one();
          was_poped = true;
        }

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

    std::unique_lock lock(mutex);
    cv_push.wait(lock, [this]() {
      // if (queue.size() < size) {
      //   std::println("queue full");
      // }
      return queue.size() < capacity || end;
    });
    if (end) {
      return;
    }

    queue.push(std::move(msg));
    cv_pop.notify_one();
  }

  ~CondVarLimitedLogger() {
    end = true;
    cv_pop.notify_one();
    cv_push.notify_all();
    log_thread.join();
  }
};