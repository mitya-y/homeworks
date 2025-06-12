#include <atomic>
#include <map>
#include <vector>
#include <memory>
#include <unordered_set>
#include <utility>

class WindowSystem {
private:
  class Window {
    friend class WindowSystem;

  private:
    inline static int window_counter = 0;

    int _x {}, _y {};
    int _width {}, _heigth {};
    int _id;
    bool _opened = true;
    uint32_t _color = 0x000000ff, _bound_color = 0x00000000;

    Window(int x, int y, int width, int heigth);

    void move(int dx, int dy);
    void resize(int width, int height);

    void set_color(float r, float g, float b);
    void set_bound_color(float r, float g, float b);

    int get_id();

    bool is_intersect(int x, int y);

    void close();
    void open();

    static uint32_t parse_color(float r, float g, float b);
  };

  constexpr static int _screen_width = 800;
  constexpr static int _screen_heigth = 800;
  using Screen = uint32_t[_screen_width * _screen_heigth];

  using WindowContainer = std::vector<std::shared_ptr<Window>>;
  using WindowIter = WindowContainer::iterator;

  Screen _screen;
  std::mutex _screen_mutex {};
  std::atomic<bool> _should_exit = false;

  WindowContainer _windows;
  std::unordered_set<int> _windows_ids;

private:
  WindowSystem();

public:
  static WindowSystem & get();

  WindowSystem(WindowSystem &other) = delete;
  WindowSystem & operator=(WindowSystem &other) = delete;

  int create_window(int x, int y, int width, int heigth);
  void delete_window(int id);

  const std::unordered_set<int> & get_windows_ids();

  void move_window(int id, int dx, int dy);
  void resize_window(int id, int width, int height);
  void close_window(int id);
  void open_window(int id);
  void set_window_color(int id, float r, float g, float b);
  void set_window_bound_color(int id, float r, float g, float b);

  void click(int x, int y);

  void draw_windows();

  void display();

  void start();
  void exit();

private:
  void assert_window_exists(int id);
  WindowIter find_window_iter(int id);
  void push_on_top(WindowIter wnd);
  void push_on_bottom(WindowIter wnd);

  void set_pixel(int x, int y, uint32_t color);
};