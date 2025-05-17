#include <atomic>
#include <print>
#include <thread>
#include <vector>
#include <ranges>

using namespace std::chrono_literals;

static std::tuple<std::size_t, std::size_t, std::size_t> get_random_idx(std::size_t bound) {
  if (bound < 2) {
    throw std::runtime_error("can't get 3 unique numbers from 2 numbers");
  }

  std::size_t i = rand() % bound;

  std::size_t j = rand() % bound;
  while (j == i) {
    j = rand() % bound;
  }

  std::size_t k = rand() % bound;
  while (k == i || k == j) {
    k = rand() % bound;
  }

  return {i, j, k};
}

static std::vector<uint64_t> get_rand_array(std::uint32_t size) {
  std::vector<uint64_t> res(size, 1);
  // for (auto &i : res) {
  //   i = rand();
  // }
  return res;
}

static void print_vec(const std::vector<uint64_t> &v) {
  for (auto i : v) {
    std::print("{} ", i);
  }
  std::println();
}

static void incorrect() {
  constexpr uint32_t n = 20;
  auto nums = get_rand_array(n);

  uint32_t thread_num = 5;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto [i, j, k] = get_random_idx(n);
        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;
      }
    }
    ));
  }

  std::this_thread::sleep_for(1s);
}

static void correct_a() {
  constexpr uint32_t n = 30;
  auto nums = get_rand_array(n);

  std::mutex mutex;

  uint32_t thread_num = 3;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  auto stop = 1000;
  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (stop-- >= 0 && !stop_token.stop_requested()) {
        auto [i, j, k] = get_random_idx(n);

        std::lock_guard lock(mutex);
        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;
      }
    }));
  }

  std::this_thread::sleep_for(1ms);
  // print_vec(nums);
}

static void correct_b() {
  constexpr uint32_t n = 30;
  auto nums = get_rand_array(n);

  std::vector<std::mutex> mutexes(n);

  uint32_t thread_num = 3;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  auto stop = 10;
  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (stop-- >= 0 && !stop_token.stop_requested()) {
        auto [i, j, k] = get_random_idx(n);
        std::array idx {i, j, k};

        uint32_t locked_n = 0;
        for (auto l : idx) {
          if (!mutexes[l].try_lock()) {
            break;
          }
          locked_n++;
        }

        if (locked_n == idx.size()) {
          auto sum = nums[i] + nums[j] + nums[k];
          nums[i] = nums[j] = nums[k] = sum;
        }

        for (auto l : std::ranges::iota_view(0u, locked_n) | std::ranges::reverse_view) {
          mutexes[idx[l]].unlock();
        }
      }
    }));
  }

  std::this_thread::sleep_for(1ms);
  std::println("HELLO WORLD");
  // print_vec(nums);
}

static void correct_d() {
  uint32_t n = 30;
  auto nums = get_rand_array(n);

  std::vector<std::mutex> mutexes(n);

  uint32_t thread_num = 3;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  std::atomic_bool done = false;
  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        if (done) {
          return;
        }
        auto [i, j, k] = get_random_idx(n);
        std::array idx {i, j, k};
        std::sort(idx.begin(), idx.end());

        for (auto l : idx) {
          mutexes[l].lock();
        }

        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;

        for (auto l : std::ranges::reverse_view(idx)) {
          mutexes[l].unlock();
        }
      }
    }));
  }

  std::this_thread::sleep_for(100ns);
  done = true;
  // for (auto &th : threads) {
  //   th.join();
  // }
  // print_vec(nums);
}

int main() {
  // incorrect();
  // std::println("incorrect done");

  correct_a();
  std::println("A done");
  correct_b();
  std::println("B done");
  // correct_d();
  // std::println("D done");
}
