#include <functional>

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

  return list._first.load()->next == nullptr;
}


bool test() {
  INIT_TESTS();

  REGISTER_TEST(test_add_seq);
  REGISTER_TEST(test_check_seq);
  REGISTER_TEST(test_remove_seq);

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