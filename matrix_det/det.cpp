#include "det.hpp"

static float internal_det_thread_pool(ThreadPool<float> &pool, const MatrixT &matrix) {
  std::size_t n = matrix.size();
  std::vector<std::future<float>> futures;
  futures.reserve(n);

  // static int min_n = 100;
  // if (min_n > n) {
  //   min_n = n;
  //   std::println("min_n = {}", min_n);
  // }

  // matrix.print();

  std::latch latch(n);
  for (std::size_t i = 0; i < n; i++) {
    futures.push_back(pool.add_task([&, i]() {
      if (n == 1) {
        latch.count_down();
        return matrix.data()[0][0];
      }

      auto num = matrix.data()[0][i];
      auto minor = matrix.minor(std::size_t(0), i);
      auto minor_det = internal_det_thread_pool(pool, minor);

      latch.count_down();
      return num * minor_det * (i % 2 == 0 ? 1 : -1);
    }, false));
  }

  while (!latch.try_wait()) {
    pool.use_this_thread_for_task();
  }

  float result = 0;
  for (auto &fut : futures) {
    result += fut.get();
  }
  return result;
}

static float internal_det_thread_seq(const MatrixT &matrix) {
  auto n = matrix.size();
  if (n == 1) {
    return matrix.data()[0][0];
  }

  float result = 0;
  for (auto &&[i, num] : std::ranges::enumerate_view(matrix.data()[0])) {
    auto minor = matrix.minor(std::size_t(0), i);
    auto minor_det = internal_det_thread_seq(minor);
    result += num * minor_det * (i % 2 == 0 ? 1 : -1);
  }
  return result;
}

static float internal_det_thread_par(const MatrixT &matrix, std::size_t thread_num) {
  auto n = matrix.size();
  if (n == 1) {
    return matrix.data()[0][0];
  }

  std::vector<std::jthread> threads;
  thread_num = std::min(thread_num, n);
  threads.reserve(thread_num);

  std::atomic<float> result;
  for (auto i : std::ranges::iota_view(std::size_t(0), thread_num)) {
    threads.emplace_back([&, i]() {
      auto idx = i;
      while (idx < n) {
        auto num = matrix.data()[0][idx];
        auto minor = matrix.minor(std::size_t(0), idx);
        auto minor_det = internal_det_thread_seq(minor);
        result += num * minor_det * (idx % 2 == 0 ? 1 : -1);
        idx += thread_num;
      }
    });
  }

  for (auto &thread : threads) {
    thread.join();
  }
  return result;
}

// interface functions

float det_thread_pool(const MatrixT &matrix, std::size_t thread_num) {
  ThreadPool<float> pool(thread_num);
  auto result = pool.add_task([&]() { return internal_det_thread_pool(pool, matrix); });
  return result.get();
}

float det_threads(const MatrixT &matrix, std::size_t thread_num) {
  auto n = matrix.size();
  return internal_det_thread_par(matrix, thread_num);
}