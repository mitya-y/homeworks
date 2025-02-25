#include "det.hpp"

static float internal_det_thread_pool(ThreadPool<float> &pool, const MatrixT &matrix) {
  std::size_t n = matrix.size();
  std::vector<std::future<float>> futures;
  futures.reserve(n);

  static int min_n = 100;
  if (min_n > n) {
    min_n = n;
    std::println("min_n = {}", min_n);
  }

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

// interface functions

float det_thread_pool(const MatrixT &matrix) {
  ThreadPool<float> pool(10);
  auto result = pool.add_task([&]() { return internal_det_thread_pool(pool, matrix); });
  return result.get();
}