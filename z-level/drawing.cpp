#include <iostream>
#include <cstring>
#include <GL/glut.h>

#include "window.hpp"

void WindowSystem::display() {
  if (_should_exit) {
    ::exit(0);
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPixelZoom(1, -1);
  glRasterPos2d(-1, 1);

  _screen_mutex.lock();
  glDrawPixels(_screen_heigth, _screen_width, GL_BGRA_EXT, GL_UNSIGNED_BYTE, _screen);
  glFinish();
  _screen_mutex.unlock();

  glutSwapBuffers();
  glutPostRedisplay();
}

static void Display() {
  WindowSystem::get().display();
}

static void Mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    auto &windows = WindowSystem::get();
    windows.click(x, y);
    windows.draw_windows();
  }
}

static std::tuple<int *, char **> get_argc_argv() {
  static char arg0[] = "z-levels";
  static std::array<char *, 1> argv { arg0 };
  static int argc = argv.size();

  return std::make_tuple(&argc, argv.data());
}

WindowSystem::WindowSystem() {
  std::memset(_screen, 0xff, sizeof(uint32_t) * _screen_width * _screen_heigth);

  auto [argc, argv] = get_argc_argv();
  glutInit(argc, argv);

  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

  glutInitWindowPosition(0, 0);
  glutInitWindowSize(_screen_width, _screen_heigth);
  glutCreateWindow("windows");

  glutDisplayFunc(Display);
  glutMouseFunc(Mouse);
}

void WindowSystem::start() {
  glutMainLoop();
}

void WindowSystem::exit() {
  _should_exit = true;
}

void WindowSystem::draw_windows() {
  std::lock_guard guard(_screen_mutex);

  std::memset(_screen, 0xff, sizeof(uint32_t) * _screen_width * _screen_heigth);
  for (const auto &wnd : _windows) {
    if (!wnd->_opened) {
      continue;
    }

    int lastx = wnd->_x + wnd->_width;
    int lasty = wnd->_y + wnd->_heigth;
    for (int x = wnd->_x; x < lastx; x++) {
      for (int y = wnd->_y; y < lasty; y++) {
        uint32_t color =
          x == wnd->_x || y == wnd->_y || x == lastx - 1 || y == lasty - 1
          ? wnd->_bound_color : wnd->_color;
        set_pixel(x, y, color);
      }
    }
  }
}

void WindowSystem::set_pixel(int x, int y, uint32_t color) {
  if (x < 0 || x >= _screen_width || y < 0 || y >= _screen_heigth) {
    return;
  }
  _screen[y * _screen_heigth + x] = color;
}