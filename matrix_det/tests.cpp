#include "thread_pool.hpp"

static bool test_work() {
  std::size_t n = 10;
  ThreadPool<void> pool(n);

  std::vector<std::future<void>> futures;

  std::atomic<int> b;
  int a = 30;
  for (int i = 0; i < a; i++) {
    futures.push_back(pool.add_task([&b]() {
      b++;
    }));
  }

  for (auto &fut : futures) {
    fut.wait();
  }
  return b == a;
}

static bool test_separate() {
  ThreadPool<void> pool(2);

  int max_cnt = 100'000'000;
  std::atomic<int> cnt = 0;
  std::atomic_bool flag = false;

  auto task1 = [&]() {
    while (true) {
      if (cnt >= max_cnt) {
        break;
      }
    }
  };

  auto task2 = [&]() {
    for (int i = 0; i < max_cnt; i++) {
      cnt++;
    }
    flag = true;
  };

  pool.add_task(task1);
  pool.add_task(task2);

  return true;
}

bool test_thread_pool() {
  auto result = test_separate() && test_work();
  std::println("tests {}", result ? "pass" : "fail");
  return result;
}