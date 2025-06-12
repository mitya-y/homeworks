#include <iostream>
#include <ranges>
#include <print>
#include <charconv>
#include <thread>
#include <functional>

#include "window.hpp"

static std::vector<std::string_view> split(const std::string &line) {
  return std::views::split(line, ' ') |
         std::views::transform([](auto w) {
           return std::string_view(w.begin(), w.end());
         }) |
         std::ranges::to<std::vector>();
}

template <typename T>
static T parse_scalar(std::string_view s) {
  T value;
  auto [_, ec] = std::from_chars(s.data(), s.data() + s.length(), value);

  if (ec == std::errc::invalid_argument) {
    throw std::runtime_error("This is not a number");
  } else if (ec == std::errc::result_out_of_range) {
    throw std::runtime_error("This number is larger than an T");
  }

  return value;
}

static void parse_command(WindowSystem &windows) {
  std::string line;
  std::getline(std::cin, line);
  auto splitted = split(line);

  if (splitted.size() < 1) {
    throw std::runtime_error("invalid string");
  }
  std::string_view command = splitted[0];

  if (command == "add") {
    if (splitted.size() < 5) {
      throw std::runtime_error("invalid string");
    }
    auto x = parse_scalar<int>(splitted[1]);
    auto y = parse_scalar<int>(splitted[2]);
    auto w = parse_scalar<int>(splitted[3]);
    auto h = parse_scalar<int>(splitted[4]);
    int id = windows.create_window(x, y, w, h);
    std::println("id: {}", id);
  } else if (command == "list") {
    const auto &list = windows.get_windows_ids();
    for (int id : list) {
      std::cout << id << ' ';
    }
    std::cout << std::endl;
  } else if (command == "delete") {
    if (splitted.size() < 2) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    windows.delete_window(id);
  } else if (command == "click") {
    if (splitted.size() < 3) {
      throw std::runtime_error("invalid string");
    }
    auto x = parse_scalar<int>(splitted[1]);
    auto y = parse_scalar<int>(splitted[2]);
    windows.click(x, y);
  } else if (command == "close") {
    if (splitted.size() < 2) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    windows.close_window(id);
  } else if (command == "open") {
    if (splitted.size() < 2) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    windows.open_window(id);
  } else if (command == "move") {
    if (splitted.size() < 4) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    auto dx = parse_scalar<int>(splitted[2]);
    auto dy = parse_scalar<int>(splitted[3]);
    windows.move_window(id, dx, dy);
  } else if (command == "resize") {
    if (splitted.size() < 4) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    auto w = parse_scalar<int>(splitted[2]);
    auto h = parse_scalar<int>(splitted[3]);
    windows.resize_window(id, w, h);
  } else if (command == "setcolor") {
    if (splitted.size() < 5) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    auto r = parse_scalar<float>(splitted[2]);
    auto g = parse_scalar<float>(splitted[3]);
    auto b = parse_scalar<float>(splitted[4]);
    windows.set_window_color(id, r, g, b);
  } else if (command == "setbound") {
    if (splitted.size() < 5) {
      throw std::runtime_error("invalid string");
    }
    auto id = parse_scalar<int>(splitted[1]);
    auto r = parse_scalar<float>(splitted[2]);
    auto g = parse_scalar<float>(splitted[3]);
    auto b = parse_scalar<float>(splitted[4]);
    windows.set_window_bound_color(id, r, g, b);
  } else if (command == "exit") {
    windows.exit();
    exit(0);
  } else {
    std::println("this command is not exist");
  }
}

static void update(WindowSystem &windows) {
  while (true) {
    try {
      parse_command(windows);
    } catch (std::runtime_error e) {
      std::cout << e.what() << std::endl;
    }
    windows.draw_windows();
  }
}

int main(int argc, char **argv) {
  auto &windows = WindowSystem::get();
  std::jthread update_loop([&windows]() { update(windows); } );
  windows.start();
}