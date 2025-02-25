#include "det.hpp"

int main() {
  // return test_thread_pool();

  auto matrix = MatrixT("");
  assert(matrix.size() == matrix.data()[0].size());

  // std::println("det = {}", det_thread_pool(matrix));
  std::println("det = {}", det_threads(matrix));
}