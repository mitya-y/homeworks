#include <print>
#include <assert.h>

#include "thread_pool.hpp"

class MatrixT {
  std::vector<std::vector<float>> _matrix;

public:
  MatrixT() {}

  MatrixT(std::string_view filename) {
    _matrix = {
      {1, 0, 0},
      {0, 1, 0},
      {0, 0, 1}
    };
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

  matrix.print();

  for (std::size_t i = 0; i < n; i++) {
    futures.push_back(pool.add_task([&]() {
      if (n == 1) {
        return matrix.data()[0][0];
      }

      auto num = matrix.data()[0][i];
      auto minor = matrix.minor(std::size_t(0), i);
      auto minor_det = det(pool, minor);
      return num * minor_det;
    }));
  }

  // return pool.add_task([futures = futures]() {
  //   float result = 0;
  //   for (auto &fut : futures) {
  //     result += fut.get();
  //   }
  //   return result;
  // });

  float result = 0;
  for (auto &fut : futures) {
    result += fut.get();
  }
  return result;
}

int main() {
  auto matrix = MatrixT("");
  // assert(matrix.size() == matrix[0].size());

  ThreadPool<float> pool(1);

  std::vector<std::size_t> wrong_cols;
  auto result = det(pool, matrix);
  std::println("det = {}", result);
}