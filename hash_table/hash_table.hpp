#pragma once

#include <print>
#include <mutex>
#include <shared_mutex>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>

using namespace std::literals::chrono_literals;

struct Dummy {
  std::string s;
  int d;

  bool operator==(const Dummy &other) const {
    return s == other.s && d == other.d;
  }
};

class HashTable {
private:
public:
  struct TableList {
    struct Node {
      std::atomic<Node *> next {};
      Dummy value;
      std::mutex mutex;

      Node(Dummy &&value) : value(std::move(value)) {}
      Node(const Dummy &value) : value(value) {}

      Node() = default;
    };

    Node _first_node_data;
    std::atomic<Node *> _first {}, _last {};
    mutable std::mutex add_mutex;

    std::atomic<uint32_t> _size = 0;

    TableList();
    ~TableList();

    void add(Dummy &&value);
    void add(const Dummy &value);

    bool check(Dummy &&value) const;
    bool check(const Dummy &value) const;

    void remove(Dummy &&value);
    void remove(const Dummy &value);

    uint32_t size() const { return _size.load(); }
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

bool run_tests();