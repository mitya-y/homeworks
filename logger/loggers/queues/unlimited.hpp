#include <queue>
#include <mutex>
#include <condition_variable>

#include "msg.hpp"

class UnlimitedQueue {
private:
  std::queue<Message> queue;
  std::mutex mutex;
  std::condition_variable cv;

public:
  UnlimitedQueue() = default;

  void push(Message &&msg) {
    std::lock_guard(mutex);
    queue.push(std::move(msg));
    cv.notify_one();
  }

  bool pop(Message &msg) {
    std::unique_lock lock(mutex);
    cv.wait(lock, [this]() {
      return !queue.empty();
    });

    if (queue.empty()) {
      return false;
    }
    msg = std::move(queue.front());
    queue.pop();
    return true;
  }
};