#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class CondVarLimitedLogger : public Logger {
private:
  class LimitedQueue {
  private:
    std::queue<Message> queue;
    std::mutex mutex;
    std::condition_variable cv_pop;
    std::condition_variable cv_push;
    std::size_t size;
    std::atomic_bool done = false;

  public:
    LimitedQueue(std::size_t size) : size(size) {}

    void push(Message &&msg) {
      std::unique_lock lock(mutex);
      cv_push.wait(lock, [this]() {
        // if (queue.size() < size) {
        //   std::println("queue full");
        // }
        return queue.size() < size || done;
      });

      queue.push(std::move(msg));
      cv_pop.notify_one();
    }

    bool pop(Message &msg) {
      std::unique_lock lock(mutex);
      cv_pop.wait(lock, [this]() {
        return !queue.empty() || done;
      });

      if (queue.empty()) {
        return false;
      }
      msg = std::move(queue.front());
      queue.pop();
      cv_push.notify_one();
      return true;
    }

    ~LimitedQueue() {
      done = true;
      cv_push.notify_one();
      cv_pop.notify_one();
    }
  };
  LimitedQueue queue;

public:
  CondVarLimitedLogger(std::ostream &out, std::size_t size = 10) : Logger(out), queue(size) {
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