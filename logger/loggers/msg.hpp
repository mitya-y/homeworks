#pragma once

#include <string_view>
#include <cstring>

struct Message {
  char *data = nullptr;
  std::size_t size = 0;

  Message() = default;

  Message(std::string_view msg) {
    size = msg.size();
    data = new char[size];
    strncpy(data, msg.data(), size);
  }

  ~Message() {
    delete data;
    data = nullptr;
    size = 0;
  }

  Message(const Message &other) = delete;
  Message & operator=(const Message &other) = delete;

  Message(Message &&other) {
    std::swap(data, other.data);
    std::swap(size, other.size);
  }

  Message & operator=(Message &&other) {
    if (this == &other) {
      return *this;
    }
    std::swap(data, other.data);
    std::swap(size, other.size);
    return *this;
  }
};