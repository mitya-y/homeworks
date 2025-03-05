#include <functional>
#include <numeric>
#include <random>

#include "hash_table.hpp"

using TestCaseT = std::pair<std::function<bool()>, std::string_view>;
#define TEST_VECTOR_NAME test_cases
#define INIT_TESTS() std::vector<TestCaseT> TEST_VECTOR_NAME;
#define REGISTER_TEST(test_name) TEST_VECTOR_NAME.push_back({test_name, #test_name});

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

  uint32_t n_per_thread = 347;
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


bool test() {
  INIT_TESTS();

  REGISTER_TEST(test_add_seq);
  REGISTER_TEST(test_check_seq);
  REGISTER_TEST(test_remove_seq);

  REGISTER_TEST(test_add);
  REGISTER_TEST(test_check);
  REGISTER_TEST(test_remove);

  auto test_result = true;
  for (auto &&[test_case, name] : TEST_VECTOR_NAME) {
    bool result = test_case();
    std::println("{} {}", name, result ? "passed" : "failed");
    if (!result) {
        test_result = result;
    }
  }

  return test_result;
}