#if 0
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

  public:
    LimitedQueue(std::size_t size) : size(size) {}

    void push(Message &&msg) {
      std::unique_lock lock(mutex);
      cv_push.wait(lock, [this]() {
        return queue.size() < size;
      });

      queue.push(std::move(msg));
      cv_pop.notify_one();
    }

    bool pop(Message &msg) {
      std::unique_lock lock(mutex);
      cv_pop.wait(lock, [this]() {
        return !queue.empty();
      });

      if (queue.empty()) {
        return false;
      }
      msg = std::move(queue.front());
      queue.pop();
      cv_push.notify_one();
      return true;
    }
  };
  LimitedQueue queue;

public:
  CondVarLimitedLogger(std::ostream &out) : Logger(out) {
    thread = std::jthread([this](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        Message msg;
        if (queue.pop(msg)) {
        }
      }
    });
  }

  void log(std::string_view msg) override {
  }
};
#endif