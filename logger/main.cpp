#include <print>
#include <fstream>
#include <chrono>

#include "cond_var_limited.hpp"
#include "cond_var_unlimited.hpp"
#include "limited.hpp"
#include "unlimited.hpp"

using namespace std::chrono_literals;

template<typename LoggerT>
static void test_cond_var_unlimited(bool random_delay = false) {
  std::ofstream out(std::format("{}-rand_del-{}.txt", typeid(LoggerT).name(), random_delay));
  LoggerT logger(out);

  uint32_t n = 12;
  std::vector<std::jthread> threads;
  threads.reserve(n);

  for (uint32_t th = 0; th < n; th++) {
    threads.emplace_back([&logger, th](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto msg = std::format("hello from {} thread\n", th);
        logger.log(msg);
        std::this_thread::sleep_for(10ns);
        // std::chrono::nanoseconds(10);
      }
    });
  }

  std::this_thread::sleep_for(100ms);

  std::println("{} done", typeid(LoggerT).name());
}

int main() {
  test_cond_var_unlimited<CondVarUnlimitedLogger>();
  test_cond_var_unlimited<CondVarLimitedLogger>();
}
