#include "thread_pool.hpp"
#include "matrix.hpp"

float det_thread_pool(const MatrixT &matrix, std::size_t thread_num = 10);

float det_threads(const MatrixT &matrix, std::size_t thread_num = 10);

bool test();