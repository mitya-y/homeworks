#include "hash_table.hpp"

using namespace std::literals::chrono_literals;

HashTable::TableList::TableList() {
  _first = _last = new Node({});
}

HashTable::TableList::~TableList() {

  Node *first = _first.load();
  first->mutex.lock();
  while (first != nullptr) {
    elements.push_back(first);
    Node *next = first->next.load();
    if (next != nullptr) {
    }
  }
}

void HashTable::TableList::add(Dummy &&value) {
  Node *last = nullptr;
  while (true) {
    // todo: maybe rename it to "add_mutex"
    add_mutex.lock();
    // this lines are seq
    last = _last.load();
    if (last->mutex.try_lock()) {
      break;
    }
    add_mutex.unlock();

    std::this_thread::sleep_for(1ns);
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
  // first always valid
  Node *prev = _first.load();

  bool result = true;
  prev->mutex.lock();
  while (true) {
    Node *next = prev->next.load();
    if (next == nullptr) {
      result = false;
      break;
    }

    next->mutex.lock();
    prev->mutex.unlock();
    prev = next;

    if (next->value == value) {
      result = true;
      break;
    }
  }
  prev->mutex.unlock();

  return result;
}

bool HashTable::TableList::check(const Dummy &value) const {
  auto dummy_copy = value;
  return check(std::move(dummy_copy));
}

void HashTable::TableList::remove(Dummy &&value) {
  // first always valid
  Node *prev = _first.load();

  prev->mutex.lock();
  while (true) {
    Node *next = prev->next.load();
    if (next == nullptr) {
      break;
    }

    next->mutex.lock();

    if (next->value == value) {
      prev->next.store(next->next.load());
      // bruh it doesnt work... other thread can try to use its mutex...
      // but maybe no? maybe it try first use prev's mutex?
      next->mutex.unlock();
      delete next;
      break;
    }

    prev->mutex.unlock();
    prev = next;
  }

  prev->mutex.unlock();
}

void HashTable::TableList::remove(const Dummy &value) {
  auto dummy_copy = value;
  remove(std::move(dummy_copy));
}
