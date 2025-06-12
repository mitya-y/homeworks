#include <algorithm>
#include <limits>

#include "window.hpp"

WindowSystem::Window::Window(int x, int y, int width, int heigth) {
  _x = x;
  _y = y;
  _width = width;
  _heigth = heigth;

  _id = window_counter++;
}

void WindowSystem::Window::move(int dx, int dy) {
  _x += dx;
  _y += dy;
}

void WindowSystem::Window::resize(int width, int height) {
  _width = width;
  _heigth = height;
}


int WindowSystem::Window::get_id() {
  return _id;
}

bool WindowSystem::Window::is_intersect(int x, int y) {
  int left = _x, right = _x + _width;
  int top = _y, bottom = _y + _heigth;
  return x >= left && x < right && y >= top && y < bottom;
}

void WindowSystem::Window::close() {
  _opened = false;
}

void WindowSystem::Window::open() {
  _opened = true;
}


uint32_t WindowSystem::Window::parse_color(float r, float g, float b) {
  r = std::clamp(r, 0.f, 1.f);
  g = std::clamp(g, 0.f, 1.f);
  b = std::clamp(b, 0.f, 1.f);
  uint8_t
    rb = static_cast<uint8_t>(r * std::numeric_limits<uint8_t>::max()),
    gb = static_cast<uint8_t>(g * std::numeric_limits<uint8_t>::max()),
    bb = static_cast<uint8_t>(b * std::numeric_limits<uint8_t>::max());
  return (rb << 16) | (gb << 8) | bb;
}

void WindowSystem::Window::set_color(float r, float g, float b) {
  _color = parse_color(r, g, b);
}

void WindowSystem::Window::set_bound_color(float r, float g, float b) {
  _bound_color = parse_color(r, g, b);
}

///////////////////////////////////////////////////

int WindowSystem::create_window(int x, int y, int width, int heigth) {
  _windows.push_back(std::shared_ptr<Window>(new Window(x, y, width, heigth)));
  _windows_ids.insert(_windows.back()->get_id());
  return _windows.back()->get_id();
}

void WindowSystem::delete_window(int id) {
  assert_window_exists(id);
  _windows_ids.erase(_windows_ids.find(id));
  _windows.erase(find_window_iter(id));
}

const std::unordered_set<int> & WindowSystem::get_windows_ids() {
  return _windows_ids;
}

void WindowSystem::move_window(int id, int dx, int dy) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->move(dx, dy);
  push_on_top(wnd);
}

void WindowSystem::resize_window(int id, int width, int height) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->resize(width, height);
  push_on_top(wnd);
}

void WindowSystem::close_window(int id) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->close();
  push_on_bottom(wnd);
}

void WindowSystem::open_window(int id) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->open();
  push_on_top(wnd);
}

void WindowSystem::set_window_color(int id, float r, float g, float b) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->set_color(r, g, b);
}

void WindowSystem::set_window_bound_color(int id, float r, float g, float b) {
  assert_window_exists(id);
  auto wnd = find_window_iter(id);
  (*wnd)->set_bound_color(r, g, b);
}

void WindowSystem::click(int x, int y) {
  for (int i = _windows.size() - 1; i >= 0; i--) {
    if (_windows[i]->is_intersect(x, y)) {
      push_on_top(_windows.begin() + i);
      return;
    }
  }
}

void WindowSystem::assert_window_exists(int id) {
  if (!_windows_ids.contains(id)) {
    throw std::runtime_error("window not exists");
  }
}

WindowSystem::WindowIter WindowSystem::find_window_iter(int id) {
  return std::ranges::find_if(_windows,
    [id](const std::shared_ptr<Window> &wnd) { return wnd->get_id() == id; });
}

void WindowSystem::push_on_top(WindowIter wnd) {
  size_t index = std::distance(_windows.begin(), wnd);
  auto save = *wnd;
  for (int i = index; i < _windows.size() - 1; i++) {
    _windows[i] = _windows[i + 1];
  }
  _windows.back() = save;
}

void WindowSystem::push_on_bottom(WindowIter wnd) {
  size_t index = std::distance(_windows.begin(), wnd);
  auto save = *wnd;
  for (int i = index; i > 0; i--) {
    _windows[i] = _windows[i - 1];
  }
  _windows.front() = save;
}

WindowSystem & WindowSystem::get() {
  static WindowSystem windows;
  return windows;
}