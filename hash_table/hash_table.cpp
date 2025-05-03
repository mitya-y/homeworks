#include "hash_table.hpp"

using namespace std::literals::chrono_literals;

HashTable::TableList::TableList() {
  _first = _last = &_first_node_data;
}

HashTable::TableList::TableList(TableList &&other) noexcept {
  _size.store(other._size.load());
  _first_node_data.next.store(other._first_node_data.next.load());
  _first = &_first_node_data;
  _last = other._last.load();

  other._size.store(0);
  other._first_node_data.next.store(nullptr);
  other._first = other._last = &other._first_node_data;
}

HashTable::TableList & HashTable::TableList::operator=(TableList &&other) noexcept {
  if (this == &other) {
    return *this;
  }

  _size.store(other._size.load());
  _first_node_data.next.store(other._first_node_data.next.load());
  _first = &_first_node_data;
  _last = other._last.load();

  other._size.store(0);
  other._first_node_data.next.store(nullptr);
  other._first = other._last = &other._first_node_data;

  return *this;
}

HashTable::TableList::~TableList() {
  Node *next = _first.load()->next.load();
  while (next != nullptr) {
    Node *old = next;
    next = next->next.load();
    delete old;
  }

  return;
  Node *first = _first.load();
  first->mutex.lock();

  std::vector<Node *> elements;
  while (first != nullptr) {
    Node *next = first->next.load();
    if (next == nullptr) {
      break;
    }
    next->mutex.lock();
    elements.push_back(next);
    first = next;
  }

  while (elements.size() > 0) {
    Node *last = elements.back();
    elements.pop_back();
    last->mutex.unlock();
    delete last;
  }

  first->mutex.unlock();
}

void HashTable::TableList::add(Dummy &&value) {
  Node *last = nullptr;
  while (true) {
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
  _size++;
  _last.store(last->next);

  last->mutex.unlock();
  add_mutex.unlock();
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
      // i think this line is neccessory, but works without it...
      std::lock_guard lock(add_mutex);

      prev->next.store(next->next.load());
      next->mutex.unlock();
      if (next == _last.load()) {
        _last.store(prev);
      }
      delete next;
      _size--;
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

HashTable::HashTable(std::size_t hashtable_size) {
  _lists.reserve(hashtable_size);
  for (std::size_t i = 0; i < hashtable_size; i++) {
    _lists.emplace_back();
  }
}

void HashTable::add(Dummy &&value) {
  auto hash = std::hash<Dummy>{}(value) % _lists.size();
  _lists[hash].add(std::move(value));
}

void HashTable::add(const Dummy &value) {
  auto dummy_copy = value;
  add(std::move(dummy_copy));
}

bool HashTable::check(Dummy &&value) {
  auto hash = std::hash<Dummy>{}(value) % _lists.size();
  return _lists[hash].check(std::move(value));
}

bool HashTable::check(const Dummy &value) {
  auto dummy_copy = value;
  return check(std::move(dummy_copy));
}

void HashTable::remove(Dummy &&value) {
  auto hash = std::hash<Dummy>{}(value) % _lists.size();
  _lists[hash].remove(std::move(value));
}

void HashTable::remove(const Dummy &value) {
  auto dummy_copy = value;
  remove(std::move(dummy_copy));
}

#if 0
void HashTable::TableList::add(Dummy &&value) {
  Node *last = nullptr;
  std::unique_lock lock(add_mutex);
  cond_var.wait(lock, [&]() {
    // if this condition will not satisfied, this thread can sleep on forever,
    // or remove and add must call cv.nptify_one()
    last = _last.load();
    return last->mutex.try_lock();
  });

  last->next = new Node(std::move(value));
  _size++;
  _last.store(last->next);

  last->mutex.unlock();

  lock.unlock();
  cond_var.notify_one();
}
#endif