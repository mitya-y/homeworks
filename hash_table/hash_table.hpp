#pragma once

#include <print>
#include <mutex>
#include <vector>
#include <atomic>
#include <chrono>

struct Dummy {
  std::string s;
  int d;

  bool operator==(const Dummy &other) const {
    return s == other.s && d == other.d;
  }
};

class HashTable {
private:
  struct TableList {
    struct Node {
      std::atomic<Node *> next {};
      Dummy value;
      std::mutex mutex;

      Node(Dummy &&value) : value(std::move(value)) {}
      Node(const Dummy &value) : value(value) {}
    };

    std::atomic<Node *> _first {}, _last {};
    std::mutex mutex;

    TableList();

    void add(Dummy &&value);
    void add(const Dummy &value);

    bool check(Dummy &&value) const;
    bool check(const Dummy &value) const;

    void remove(Dummy &&value);
    void remove(const Dummy &value);
  };

  std::vector<TableList> _lists;

public:
  HashTable(std::size_t hashtable_size = 10000);

  void add(Dummy &&value);
  void add(const Dummy &value);

  Dummy & find(Dummy &&value);
  Dummy & find(const Dummy &value);

  bool remove(Dummy &&value);
  bool remove(const Dummy &value);
};