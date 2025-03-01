#include "hash_table.hpp"

using namespace std::literals::chrono_literals;

HashTable::TableList::TableList() {
  _first = _last = new Node({});
}

void HashTable::TableList::add(Dummy &&value) {
  Node *last = nullptr;
  while (true) {
    std::lock_guard guard(mutex);
    last = _last.load();
    if (last->mutex.try_lock()) {
        break;
    }
    // todo: sleep
  }

  last->next = new Node(std::move(value));
  _last.store(last->next);

  last->mutex.unlock();
}

void HashTable::TableList::add(const Dummy &value) {
  auto dummy_copy = value;
  add(std::move(dummy_copy));
}

bool HashTable::TableList::check(Dummy &&value) const {
  auto first = _first.load();

  while (true) {
    std::lock_guard guard(first->mutex);
    // first can be deleted here
    if (first == nullptr) {
      break;
    }
    if (first->value == value) {
      return true;
    }
    first = first->next;
  }

  return false;
}

bool HashTable::TableList::check(const Dummy &value) const {
  auto dummy_copy = value;
  return check(std::move(dummy_copy));
}

void HashTable::TableList::remove(Dummy &&value) {
  auto first = _first.load();
  Node *prev = nullptr;
  // std::atomic<Node *> *prev = nullptr;

  while (true) {
    std::lock_guard guard(first->mutex);

    if (first == nullptr) {
      break;
    }
    if (first->value == value) {
      // delete
    }
  }
}

void HashTable::TableList::remove(const Dummy &value) {
  auto dummy_copy = value;
  remove(std::move(dummy_copy));
}
