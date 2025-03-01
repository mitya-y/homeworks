#include "det.hpp"

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

  // int max_cnt = 100'000'000;
  int max_cnt = 100'000;
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

template<typename T>
concept DetFuncT = std::invocable<T, const MatrixT &>;

static bool helper_test_det(DetFuncT auto &&det_func, std::size_t max_size) {
  for (int n = 1; n < max_size; n++) {
    auto data = std::vector(n, std::vector(n, 0.0f));
    for (int i = 0; i < n; i++) {
      data[i][i] = 1;
    }
    auto matrix = MatrixT(std::move(data));

    if (det_func(matrix) != 1) {
      return false;
    }
  }

  float det = -53016;
  int n = 4;
  auto data = std::vector(n, std::vector(n, 0.0f));
  data = {
    {1, 11, 43, 87},
    {3, 0, 1, 4},
    {5, 47, 0, 1},
    {11, 12, 3, 4},
  };

  if (det_func(MatrixT(std::move(data))) != det) {
    return false;
  }

  det = 25;
  n = 3;
  data = {
    {1, 3, 5},
    {4, 0, 1},
    {2, 1, 0},
  };

  if (det_func(MatrixT(std::move(data))) != det) {
    return false;
  }

  return true;
}

static bool test_det_thread_pool() {
  auto wrapper = [](const MatrixT &matrix) { return det_thread_pool(matrix); };
  return helper_test_det(wrapper, 8);
}

static bool test_det_threads() {
  auto wrapper = [](const MatrixT &matrix) { return det_threads(matrix); };
  return helper_test_det(wrapper, 10);
}

bool test() {
  using TestCaseT = std::pair<std::function<bool()>, std::string_view>;

  std::vector<TestCaseT> cases {
    {test_work, "test work"},
    {test_separate, "test separate"},
    {test_det_threads, "test det threads"},
    {test_det_thread_pool, "test det thread pool"},
  };

  bool test_passed = true;
  for (auto &&[test_case, name] : cases) {
    auto result = test_case();
    std::println("CASE: {}; RESULT: {}", name, result ? "passed" : "failed");

    if (!result) {
      test_passed = false;
    }
  }

  return test_passed;
}