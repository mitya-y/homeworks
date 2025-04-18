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
  constexpr uint32_t n = 100;
  auto nums = get_rand_array(n);

  uint32_t thread_num = 5;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  std::mutex mutex;

  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto [i, j, k] = get_random_idx(n);

        std::lock_guard lock(mutex);
        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;
      }
    }));
  }

  std::this_thread::sleep_for(1s);
  // print_vec(nums);
}

static void correct_b() {
  constexpr uint32_t n = 100;
  auto nums = get_rand_array(n);

  uint32_t thread_num = 5;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  std::vector<std::mutex> mutexes(n);

  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
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

        for (auto l : std::ranges::iota_view(0u, locked_n)) {
          mutexes[l].unlock();
        }
      }
    }));
  }

  std::this_thread::sleep_for(10ns);
  // print_vec(nums);
}

static void correct_d() {
  uint32_t n = 4;
  auto nums = get_rand_array(n);

  uint32_t thread_num = 2;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  std::vector<std::mutex> mutexes(n);

  std::atomic<std::size_t> iter_id = 0;
  for (uint32_t thr = 0; thr < thread_num; thr++) {
    threads.emplace_back(std::jthread([&, thr](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        auto [i, j, k] = get_random_idx(n);
        std::array idx {i, j, k};
        std::sort(idx.begin(), idx.end());

        auto index_str = std::format("[{} {} {}]#{}", idx[0], idx[1], idx[2], iter_id++);

        for (auto l : idx) {
          std::println("{}: wait {} from {}", thr, l, index_str);
          mutexes[l].lock();
          std::println("{}: locked {} from {}", thr, l, index_str);
        }

        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;
        std::println("{}: nums from {} was summed", thr, index_str);

        for (auto l : std::ranges::reverse_view(idx)) {
        // for (auto l : idx) {
          mutexes[l].unlock();
          std::println("{}: unlocked {}", thr, l);
        }
      }
    }));
  }

  std::this_thread::sleep_for(1s);
  // print_vec(nums);
}


int main() {
  // incorrect();
  // std::println("incorrect done");
  // correct_a();
  // std::println("A done");
  // correct_b();
  // std::println("B done");

  correct_d();
  std::println("D done");
}