#include "thread_pool.hpp"

class MatrixT {
  std::vector<std::vector<float>> _matrix;

public:
  MatrixT() {}

  MatrixT(std::string_view filename) {
    int n = 8;
    _matrix = std::vector(n, std::vector(n, 0.0f));
    for (int i = 0; i < n; i++) {
      _matrix[i][i] = 1;
    }

    // det is -53016
    // _matrix = {
    //   {1, 11, 43, 87},
    //   {3, 0, 1, 4},
    //   {5, 47, 0, 1},
    //   {11, 12, 3, 4},
    // };

    // det is 25
    // _matrix = {
    //   {1, 3, 5},
    //   {4, 0, 1},
    //   {2, 1, 0},
    // };
  }

  std::size_t size() const {
    return _matrix.size();
  }

  MatrixT minor(std::size_t row, std::size_t col) const {
    std::size_t n = _matrix.size();
    std::vector matrix(n - 1, std::vector(n - 1, 0.0f));

    std::size_t i_idx = 0;
    for (std::size_t i = 0; i < n; i++) {
      if (i == row) {
        continue;
      }

      std::size_t j_idx = 0;
      for (std::size_t j = 0; j < n; j++) {
        if (j == col) {
          continue;
        }
        matrix[i_idx][j_idx] = _matrix[i][j];
        j_idx++;
      }
      i_idx++;
    }

    MatrixT ret;
    ret._matrix = std::move(matrix);
    return ret;
  }

  const std::vector<std::vector<float>> & data() const {
    return _matrix;
  }

  void print() const {
    for (auto &row : _matrix) {
      for (auto &num : row) {
        std::print("{}", num);
      }
      std::println();
    }
    std::println();
  }
};

static float det(ThreadPool<float> &pool, const MatrixT &matrix) {
  std::size_t n = matrix.size();
  std::vector<std::future<float>> futures;
  futures.reserve(n);

  static int min_n = 100;
  if (min_n > n) {
    min_n = n;
    std::println("min_n = {}", min_n);
  }

  // matrix.print();

  std::latch latch(n);
  for (std::size_t i = 0; i < n; i++) {
    futures.push_back(pool.add_task([&, i]() {
      if (n == 1) {
        latch.count_down();
        return matrix.data()[0][0];
      }

      auto num = matrix.data()[0][i];
      auto minor = matrix.minor(std::size_t(0), i);
      auto minor_det = det(pool, minor);

      latch.count_down();
      return num * minor_det * (i % 2 == 0 ? 1 : -1);
    }, false));
  }

  while (!latch.try_wait()) {
    pool.use_this_thread_for_task();
  }

  float result = 0;
  for (auto &fut : futures) {
    result += fut.get();
  }
  return result;
}

int main() {
  auto matrix = MatrixT("");
  assert(matrix.size() == matrix.data()[0].size());

  ThreadPool<float> pool(10);

  auto result = pool.add_task([&]() { return det(pool, matrix); }).get();
  std::println("det = {}", result);
}