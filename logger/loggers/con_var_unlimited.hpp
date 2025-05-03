#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class CondVarLimitedLogger : public Logger {
private:
  class UnlimitedQueue {
  private:
    std::queue<Message> queue;
    std::mutex mutex;
    std::condition_variable cv;

  public:
    UnlimitedQueue() = default;

    void push(Message &&msg) {
      std::lock_guard lock(mutex);
      queue.push(std::move(msg));
      // maybe unlock before notifiy?
      cv.notify_one();
    }

    bool pop(Message &msg, std::atomic_bool &done) {
      std::unique_lock lock(mutex);
      cv.wait(lock, [this, &done]() {
        return !queue.empty() || done;
      });

      if (queue.empty()) {
        return false;
      }
      msg = std::move(queue.front());
      queue.pop();
      return true;
    }
  };
  UnlimitedQueue queue;
  std::atomic_bool done = false;

public:
  CondVarLimitedLogger(std::ostream &out) : Logger(out) {
    log_thread = std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        Message msg;
        if (queue.pop(msg, done)) {
          std::lock_guard lock(log_mutex);
          out << msg.data;
        }
      }
      std::println("leave from logger therad");
    });
  }

  void log(std::string_view msg) override {
    queue.push(Message(msg));
  }

  ~CondVarLimitedLogger() {
    done = true;
    std::println("logger desturct");
  }
};