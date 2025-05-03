#include <functional>
#include <numeric>
#include <random>

#include "hash_table.hpp"

using TestCaseT = std::pair<std::function<bool()>, std::string_view>;
#define TEST_VECTOR_NAME test_cases
#define INIT_TESTS() static std::vector<TestCaseT> TEST_VECTOR_NAME;
#define REGISTER_TEST(test_name) TEST_VECTOR_NAME.push_back({test_name, #test_name});

constexpr bool _check_result = true;

static bool test_add_seq() {
  HashTable::TableList list;

  uint32_t n = 1000;
  for (int i = 0; i < n; i++) {
    list.add(Dummy {"", i});
  }

  std::vector<bool> found(n, false);

  auto node = list._first.load()->next.load();
  while (node != nullptr) {
    found[node->value.d] = true;
    node = node->next.load();
  }

  if (list.size() != n) {
    return false;
  }

  return std::all_of(found.begin(), found.end(), [](bool a){ return a; });
}

static bool test_check_seq() {
  HashTable::TableList list;

  // work of add method is validated in previous cases
  uint32_t n = 1000;
  for (int i = 0; i < n; i++) {
    list.add(Dummy {"", i});
  }

  for (int i = 0; i < n; i++) {
    if (!list.check({"", i})) {
      return false;
    }
  }

  return true;
}

static bool test_remove_seq() {
  HashTable::TableList list;

  // work of add method is validated in previous cases
  uint32_t n = 1000;
  for (int i = 0; i < n; i++) {
    list.add(Dummy {"", i});
  }

  for (int i = 0; i < n; i++) {
    list.remove({"", i});
  }

  if (list.size() != 0) {
    return false;
  }

  return list._first.load()->next == nullptr;
}

static bool test_add() {
  HashTable::TableList list;

  uint32_t n_per_thread = 347;
  uint32_t threads_num = 5;
  std::vector<std::jthread> threads;
  threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    threads.emplace_back([&list, n_per_thread, i]() {
      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        list.add({"", j});
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  auto n = n_per_thread * threads_num;
  std::vector<bool> found(n, false);

  auto node = list._first.load()->next.load();
  while (node != nullptr) {
    found[node->value.d] = true;
    // std::println("{}", node->value.d);
    node = node->next.load();
  }

  if (list.size() != n) {
    return false;
  }

  return std::all_of(found.begin(), found.end(), [](bool a){ return a; });
}

static bool test_check() {
  HashTable::TableList list;

  uint32_t n_per_thread = 347;
  uint32_t threads_num = 5;
  uint32_t n = n_per_thread * threads_num;
  uint32_t n2 = n / 2;

  // work of add method is validated in previous cases
  for (int i = 0; i < n2; i++) {
    list.add(Dummy {"", i});
  }

  std::vector<int> numbers(n);
  std::iota(numbers.begin(), numbers.end(), 0);
  std::shuffle(numbers.begin(), numbers.end(), std::mt19937{});

  std::atomic_bool result = true;

  std::vector<std::jthread> threads;
  threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    threads.emplace_back([&list, &numbers, &result, n_per_thread, n2, i]() {
      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        bool contains = list.check({"", numbers[j]});
        bool expected = numbers[j] < n2;
        if (contains != expected) {
          result = false;
          break;
        }
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  return result;
}

static bool test_remove() {
  HashTable::TableList list;

  uint32_t n_per_thread = 147;
  uint32_t threads_num = 5;
  uint32_t n = n_per_thread * threads_num;

  // work of add method is validated in previous cases
  for (int i = 0; i < n; i++) {
    list.add(Dummy {"", i});
  }

  std::vector<int> numbers(n);
  std::iota(numbers.begin(), numbers.end(), 0);
  std::shuffle(numbers.begin(), numbers.end(), std::mt19937{});

  std::vector<std::jthread> threads;
  threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    threads.emplace_back([&list, &numbers, n_per_thread, i]() {
      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        list.remove({"", numbers[j]});
      }
    });
  }

  for (auto &th : threads) {
    th.join();
  }

  if (list.size() != 0) {
    return false;
  }

  return list._first.load()->next.load() == nullptr;
}

static bool test_add_check() {
  HashTable::TableList list;

  std::atomic_bool start = false;

  uint32_t n_per_thread = 147;
  uint32_t threads_num = 3;
  uint32_t n = n_per_thread * threads_num;

  std::vector<int> add_numbers(n);
  std::iota(add_numbers.begin(), add_numbers.end(), 0);
  std::shuffle(add_numbers.begin(), add_numbers.end(), std::mt19937{});

  std::vector<int> check_numbers = add_numbers;
  std::shuffle(check_numbers.begin(), check_numbers.end(), std::mt19937{});

  std::vector<std::atomic_bool> added(n);
  for (auto &b : added) {
    b = false;
  }

  std::vector<std::jthread> add_threads;
  add_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    add_threads.emplace_back([&start, &list, &added, &add_numbers, n_per_thread, i]() {
      while (!start.load()) {} // wait to start

      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        list.add({"", add_numbers[j]});
        added[add_numbers[j]] = true;
      }
    });
  }

  bool wait_before = false;

  if (wait_before) {
    start = true;
    for (auto &th : add_threads) {
      th.join();
    }
  }

  std::atomic_bool result = true;

  uint32_t runs_number = 2;
  std::vector<std::jthread> check_threads;
  check_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    check_threads.emplace_back([&start, &list, &added, &check_numbers, &result,
                              n_per_thread, i,  runs_number]() {
      while (!start.load()) {} // wait to start

      for (uint32_t run = 0; run < runs_number; run++) {
        for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
          bool contains = list.check({"", check_numbers[j]});
          bool expected = added[check_numbers[j]];

          if (_check_result && contains != expected) {
            std::println("{} {} {}", check_numbers[j], contains, expected);
            result = false;
            break;
          }
        }
      }
    });
  }

  if (!wait_before) {
    start = true;
    for (auto &th : add_threads) {
      th.join();
    }
  }

  for (auto &th : check_threads) {
    th.join();
  }

  return result;
}

static bool test_add_remove() {
  HashTable::TableList list;

  std::atomic_bool start = false;

  uint32_t n_per_thread = 147;
  uint32_t threads_num = 3;
  uint32_t n = n_per_thread * threads_num;

  std::vector<int> add_numbers(n);
  std::iota(add_numbers.begin(), add_numbers.end(), 0);
  std::shuffle(add_numbers.begin(), add_numbers.end(), std::mt19937{});

  std::vector<int> remove_numbers = add_numbers;
  std::shuffle(remove_numbers.begin(), remove_numbers.end(), std::mt19937{});

  std::vector<std::atomic_bool> added(n);
  for (auto &b : added) {
    b = false;
  }

  std::vector<std::jthread> add_threads;
  add_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    add_threads.emplace_back([&start, &list, &add_numbers, &added, n_per_thread, i]() {
      while (!start.load()) {} // wait to start

      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        list.add({"", add_numbers[j]});
        added[add_numbers[j]] = true;
      }
    });
  }

  std::atomic<uint32_t> expected_size = n;

  std::vector<std::jthread> remove_threads;
  remove_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    remove_threads.emplace_back([&start, &list, &remove_numbers, &expected_size, &added,
                                 n_per_thread, i]() {
      while (!start.load()) {} // wait to start

      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        if (added[remove_numbers[j]]) {
          expected_size--;
        }
        list.remove({"", remove_numbers[j]});
      }
    });
  }

  start = true;
  for (auto &th : add_threads) {
    th.join();
  }
  for (auto &th : remove_threads) {
    th.join();
  }

  if (_check_result) {
    if (expected_size.load() != list.size()) {
      std::println("size = {}, expected = {}", list.size(), expected_size.load());
      return false;
    }
  }
  return true;
}

static bool test_remove_check() {
  HashTable::TableList list;

  std::atomic_bool start = false;

  uint32_t n_per_thread = 147;
  uint32_t threads_num = 3;
  uint32_t n = n_per_thread * threads_num;

  // work of add method is validated in previous cases
  for (int i = 0; i < n; i++) {
    list.add(Dummy {"", i});
  }

  std::vector<int> check_numbers(n);
  std::iota(check_numbers.begin(), check_numbers.end(), 0);
  std::shuffle(check_numbers.begin(), check_numbers.end(), std::mt19937{});

  std::vector<int> remove_numbers = check_numbers;
  std::shuffle(remove_numbers.begin(), remove_numbers.end(), std::mt19937{});

  std::vector<std::atomic_bool> deleted(n);
  for (auto &b : deleted) {
    b = false;
  }

  std::vector<std::jthread> remove_threads;
  remove_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    remove_threads.emplace_back([&start, &list, &remove_numbers, &deleted, n_per_thread, i]() {
      while (!start.load()) {} // wait to start

      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        list.remove({"", remove_numbers[j]});
        deleted[remove_numbers[j]] = true;
      }
    });
  }

  std::atomic_bool result = true;
  std::vector<std::jthread> check_threads;
  check_threads.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    check_threads.emplace_back([&start, &list, &check_numbers, &deleted, &result,
                                n_per_thread, i]() {
      while (!start.load()) {} // wait to start

      for (int j = n_per_thread * i; j < n_per_thread * (i + 1); j++) {
        bool contains = list.check({"", check_numbers[j]});
        bool expected = !deleted[check_numbers[j]];

        if (_check_result && contains != expected) {
          std::println("{} {} {}", check_numbers[j], contains, expected);
          result = false;
          break;
        }
      }
    });
  }

  start = true;
  for (auto &th : check_threads) {
    th.join();
  }
  for (auto &th : remove_threads) {
    th.join();
  }

  return true;
}

bool run_tests() {
  INIT_TESTS();

  REGISTER_TEST(test_add_seq);
  REGISTER_TEST(test_check_seq);
  REGISTER_TEST(test_remove_seq);

  REGISTER_TEST(test_add);
  REGISTER_TEST(test_check);
  REGISTER_TEST(test_remove);

  REGISTER_TEST(test_add_check);
  REGISTER_TEST(test_add_remove);
  REGISTER_TEST(test_remove_check);

  auto test_result = true;
  for (auto &&[test_case, name] : TEST_VECTOR_NAME) {
    bool result = test_case();
    if (!result) {
      test_result = false;
    }

    std::string_view
      passed_string = "\x1B[32m" "passed" "\x1B[0m",
      failed_string = "\x1B[31m" "failed" "\x1B[0m";
    std::println("{} {}", name, result ? passed_string : failed_string);
  }

  return test_result;
}