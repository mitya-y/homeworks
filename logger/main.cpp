#include <print>

#include "con_var_limited.hpp"
#include "con_var_unlimited.hpp"
#include "limited.hpp"
#include "unlimited.hpp"

using namespace std::chrono_literals;

static void test_cond_var_unlimited() {
  CondVarUnlimitedLogger logger(std::cout);

  uint32_t n = 6;
  std::vector<std::jthread> threads;
  threads.reserve(n);

  for (uint32_t th = 0; th < n; th++) {
    threads.emplace_back([&logger, th](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto msg = std::format("hello from {} thread\n", th);
        logger.log(msg);
        std::this_thread::sleep_for(10ns);
      }
    });
  }

  std::this_thread::sleep_for(100ms);
}

int main() {
  test_cond_var_unlimited();
}
