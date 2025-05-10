#include <print>
#include <fstream>
#include <chrono>

#include "cond_var_limited.hpp"
#include "cond_var_unlimited.hpp"
#include "limited.hpp"
#include "unlimited.hpp"

using namespace std::chrono_literals;

template<typename LoggerT>
static void test_logger(bool random_delay = false) {
  std::ofstream out(std::format("{}-rand_del-{}.txt", typeid(LoggerT).name(), random_delay));
  LoggerT logger(out);

  uint32_t n = 12;
  std::vector<std::jthread> threads;
  threads.reserve(n);

  for (uint32_t th = 0; th < n; th++) {
    threads.emplace_back([&logger, th, random_delay](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto msg = std::format("hello from {} thread\n", th);
        logger.log(msg);
        auto delay = std::chrono::nanoseconds(random_delay ? (rand() % 100) : 10);
        std::this_thread::sleep_for(delay);
      }
    });
  }

  std::this_thread::sleep_for(100ms);

  std::println("{} with{} random delay done", typeid(LoggerT).name(), random_delay ? "" : "out");
}

using Loggers = std::tuple<
  CondVarUnlimitedLogger,
  CondVarLimitedLogger
>;

template<std::size_t index = 0>
void test() {
  if constexpr (index < std::tuple_size<Loggers>::value) {
    using T = std::tuple_element_t<index, Loggers>;
    test_logger<T>(false);
    test_logger<T>(true);
    test<index + 1>();
  }
}

int main() {
  test();
}
