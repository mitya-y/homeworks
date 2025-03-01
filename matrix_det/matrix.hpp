#pragma once

#include <vector>
#include <print>
#include <string_view>

class MatrixT {
  std::vector<std::vector<float>> _matrix;

  MatrixT() {}

public:
  MatrixT(std::string_view filename) {
    int n = 9;
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

  MatrixT(const std::vector<std::vector<float>> &data) : _matrix(data) {}
  MatrixT(std::vector<std::vector<float>> &&data) : _matrix(std::move(data)) {}

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
