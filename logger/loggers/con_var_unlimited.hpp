#include <queue>
#include <mutex>
#include <condition_variable>

#include "base.hpp"
#include "msg.hpp"

class CondVarUnlimitedLogger : public Logger {
private:
  class UnlimitedQueue {
  private:
    std::queue<Message> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic_bool done = false;

  public:
    UnlimitedQueue() = default;

    void push(Message &&msg) {
      std::lock_guard lock(mutex);
      queue.push(std::move(msg));
      // maybe unlock before notifiy?
      cv.notify_one();
    }

    bool pop(Message &msg) {
      std::unique_lock lock(mutex);
      cv.wait(lock, [this]() {
        return !queue.empty() || done;
      });

      if (queue.empty() || done) {
        return false;
      }
      msg = std::move(queue.front());
      queue.pop();
      return true;
    }

    ~UnlimitedQueue() {
      done = true;
      cv.notify_one();
    }
  };
  UnlimitedQueue queue;

public:
  CondVarUnlimitedLogger(std::ostream &out) : Logger(out) {
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