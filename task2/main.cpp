#include <print>
#include <thread>
#include <vector>
#include <ranges>

using namespace std::chrono_literals;

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

  for (uint32_t i = 0; i < thread_num; i++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        std::size_t i = rand() % n, j = rand() % n, k = rand() % n;
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

  for (uint32_t i = 0; i < thread_num; i++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        std::size_t i = rand() % n, j = rand() % n, k = rand() % n;

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

  for (uint32_t i = 0; i < thread_num; i++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        std::size_t i = rand() % n, j = rand() % n, k = rand() % n;
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
  uint32_t n = 100;
  auto nums = get_rand_array(n);

  uint32_t thread_num = 5;
  std::vector<std::jthread> threads;
  threads.reserve(thread_num);

  std::vector<std::mutex> mutexes(n);

  for (uint32_t i = 0; i < thread_num; i++) {
    threads.emplace_back(std::jthread([&](std::stop_token stop_token) {
      while (!stop_token.stop_requested()) {
        std::size_t i = rand() % n, j = rand() % n, k = rand() % n;
        std::array idx {i, j, k};
        std::sort(idx.begin(), idx.end());

        for (auto l : idx) {
          mutexes[l].lock();
        }

        auto sum = nums[i] + nums[j] + nums[k];
        nums[i] = nums[j] = nums[k] = sum;

        for (auto l : std::ranges::reverse_view(idx)) {
        // for (auto l : idx) {
          mutexes[l].unlock();
        }
      }
    }));
  }

  std::this_thread::sleep_for(1s);
  // print_vec(nums);
}


int main() {
  incorrect();
  correct_a();
  correct_b();
  // correct_d();
}