#pragma once

#include <functional>
#include <iostream>
#include <ranges>
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <print>
#include <assert.h>
#include <latch>
#include <stdexcept>
#include <list>

using namespace std::literals::chrono_literals;

#define SCOPE_FOR_LOCK_GUARD /* scope for lock guard */

template<typename RetT>
class ThreadPool {
  using FutureT = std::future<RetT>;
  using TaskFuncT = RetT();
  using TaskT = std::function<TaskFuncT>;
  using InternalTaskT = std::packaged_task<TaskFuncT>;

  std::vector<std::jthread> _threads;
  std::mutex _mutex;
  std::list<InternalTaskT> _tasks;

  bool thread_function() {
    InternalTaskT task = {};

    SCOPE_FOR_LOCK_GUARD {
      std::lock_guard lock(_mutex);
      // std::println("ts = {}", _tasks.size());
      if (_tasks.size() > 0) {
        task = std::move(_tasks.front());
        _tasks.pop_front();
      } else {
        return false;
      }
    }

    task();
    return true;
  }

public:
  ThreadPool(std::size_t num_threads) {
    if (num_threads == 0 || num_threads > 20) {
      throw std::runtime_error("invalid thread number");
    }
    auto range = std::ranges::iota_view(std::size_t(0), num_threads);

    _threads.reserve(num_threads);
    for (auto i : range) {
      _threads.emplace_back([this](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
          thread_function();
        }
      });
    }
  }

  ~ThreadPool() {
    while (true) {
      SCOPE_FOR_LOCK_GUARD {
        std::lock_guard lock(_mutex);
        if (_tasks.size() == 0) {
          break;
        }
      }
      std::this_thread::sleep_for(1ns);
    }
  }

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool & operator=(const ThreadPool &) = delete;

  FutureT add_task(const TaskT &task, bool push_back = true) {
    std::lock_guard lock(_mutex);
    _tasks.emplace_back(InternalTaskT(task));
    if (push_back) {
      _tasks.emplace_back(InternalTaskT(task));
      return _tasks.back().get_future();
    } else {
      _tasks.emplace_front(InternalTaskT(task));
      return _tasks.front().get_future();
    }
  }

  FutureT add_task(TaskT &&task, bool push_back = true) {
    std::lock_guard lock(_mutex);
    if (push_back) {
      _tasks.emplace_back(InternalTaskT(std::move(task)));
      return _tasks.back().get_future();
    } else {
      _tasks.emplace_front(InternalTaskT(std::move(task)));
      return _tasks.front().get_future();
    }
  }

  bool use_this_thread_for_task() {
    return thread_function();
  }
};

inline void test() {
  std::size_t n = 10;
  ThreadPool<void> pool(n);

  std::vector<std::future<void>> futures;

  std::atomic<int> b;
  int a = 30;
  for (int i = 0; i < a; i++) {
    futures.push_back(pool.add_task([&b]() {
      std::cout << "hello world\n";
      b++;
    }));
  }

  for (auto &fut : futures) {
    fut.wait();
  }
  std::cout << b << std::endl;
}

inline void test_separate() {
  ThreadPool<void> pool(2);

  std::atomic_bool flag = false;
  auto task1 = [&flag]() {
    while (true) {
      std::println("loop");
      if (flag) {
        break;
      }
    }
  };
  auto task2 = [&flag]() {
    for (int i = 0; i < 100'000'000; i++) {
      if (i % 1000 == 0) {
        std::println("delay");
      }
    }
    flag = true;
  };

  pool.add_task(task1);
  pool.add_task(task2);
}