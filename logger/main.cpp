#include <print>

#include "con_var_limited.hpp"
#include "con_var_unlimited.hpp"
#include "limited.hpp"
#include "unlimited.hpp"

using namespace std::chrono_literals;

static void test_cond_var_unlimited() {
  CondVarLimitedLogger logger(std::cout);

  uint32_t n = 6;
  std::vector<std::jthread> threads;
  threads.reserve(n);

  std::atomic_bool done = false;

  for (uint32_t th = 0; th < n; th++) {
    threads.emplace_back([&logger, th, &done](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        if (done) break;
        auto msg = std::format("hello from {} thread\n", th);
        logger.log(msg);
        std::this_thread::sleep_for(10ns);
      }
      std::println("desturct {} therad", th);
    });
  }

  std::this_thread::sleep_for(3s);
  // done = true;
}

int main() {
  test_cond_var_unlimited();
}
