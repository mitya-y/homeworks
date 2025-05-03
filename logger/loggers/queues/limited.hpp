#include <queue>
#include <mutex>
#include <condition_variable>

#include "msg.hpp"

class LimitedQueue {
private:
  // todo: ring buffer
  // std::vector<std::pair<Message, std::mutex>> queue;

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