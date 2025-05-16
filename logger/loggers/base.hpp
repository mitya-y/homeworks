#pragma once

#include <assert.h>
#include <iostream>
#include <thread>
#include <print>

using namespace std::chrono_literals;

class Logger {
protected:
  std::ostream &out;
  std::jthread log_thread;
  std::mutex log_mutex;

public:
  Logger(std::ostream &out) : out(out) {}

  virtual void log(std::string_view msg) = 0;
  virtual ~Logger() = default;
};