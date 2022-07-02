// This demo will allow the user to click on a window, grab the keyboard on it
// and then print every key press and release event. Will exit when Escape is
// pressed

#include <iostream>

#include <X11/Xlib.h> // XKeysymToString
#include <X11/keysym.h> // XK_Escape
#include <X11/cursorfont.h> // XC_cross

#include "../xpp.hpp"

// global variable to indicate whether the event loop should exit
bool g_quit = false;

// typedefs for convenience
namespace x {
  typedef xpp::connection<> connection;
  typedef xpp::event::registry<connection &> registry;

  typedef xpp::font<connection &> font;
  typedef xpp::cursor<connection &> cursor;
  typedef xpp::window<connection &> window;

  typedef xpp::x::event::key_press<connection &> key_press;
  typedef xpp::x::event::key_release<connection &> key_release;
  typedef xpp::x::event::button_press<connection &> button_press;
};

// The event handler class
// Implements the xpp::event::sink<..> interface with all events we are
// interested in as template parameters
template<typename Connection>
class key_printer
  : public xpp::event::sink<x::key_press, x::key_release, x::button_press>
{
  public:
    template<typename C>
    key_printer(C && c)
      : m_c(std::forward<C>(c))
    {}

    // xpp::event::sink<x::key_press>::handle(...) interface
    void handle(const x::key_press & e)
    {
      auto kbd_mapping = m_c.get_keyboard_mapping(e->detail, 1);
      // take the first value from the kbd_mapping list
      // This might throw, but for simplicity, no error handling here
      auto keysym = *kbd_mapping.keysyms().begin();

      if (keysym == XK_Escape) {
        std::cerr << "quitting" << std::endl;
        // parameter has a default value: XCB_TIME_CURRENT_TIME
        m_c.ungrab_keyboard();
        g_quit = true;
      } else {
        std::cerr << "key press: " << XKeysymToString(keysym) << std::endl;
      }
    }

    // xpp::event::sink<x::key_release>::handle(...) interface
    void handle(const x::key_release & e)
    {
      auto kbd_mapping = m_c.get_keyboard_mapping(e->detail, 1);
      auto keysym = *kbd_mapping.keysyms().begin();
      std::cerr << "key release: " << XKeysymToString(keysym) << std::endl;
    }

    // xpp::event::sink<x::button_press>::handle(...) interface
    void handle(const x::button_press & e)
    {
      m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);

      // event & reply accessors have a default template parameter, the c-type
      // Usable with any type which is constructible from the c-type or
      // connection + c-type
      // xcb_window_t grab_window = e.event();
      x::window grab_window = e.event<x::window>();

      if (e->event == e->root) {
        // xpp::window, etc. are assignable with the c-type
        grab_window = e.child();
        // xpp::window, etc. are implicitly convertible to c-type
        auto translate = grab_window.translate_coordinates(grab_window, 1, 1);
        grab_window = translate->child;
      }

      *m_c.grab_keyboard(true, grab_window,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

      std::cerr << "Grabbed " << grab_window
                << ". Press Escape to quit." << std::endl;
    }

  private:
    Connection m_c;
};

int main(int, char **)
{
  try {
    // xpp::connection is implicitly convertible to xcb_connection_t *
    // Hence, it can be used with all xcb_* c functions.
    // However, this is not demonstrated here.
    x::connection connection;
    x::registry registry(connection);

    key_printer<x::connection &> key_printer(connection);
    registry.attach(0, &key_printer);

    x::font font = x::font::open_checked(connection, "cursor");

    // x::font, etc. is implicitly convertible to xcb_font_t
    x::cursor cursor = x::cursor::create_glyph_checked(connection, font, font,
        XC_cross, XC_cross + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

    *connection.grab_pointer(false, connection.root(),
                             XCB_EVENT_MASK_BUTTON_PRESS,
                             XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                             XCB_NONE, cursor);
                             // default value for time = XCB_TIME_CURRENT_TIME);

    std::cerr << "Please click on a window" << std::endl;

    while (! g_quit) {
      connection.flush();
      registry.dispatch(connection.wait_for_event());
    }

  } catch (const std::exception & error) {
    std::cerr << "Exception (std::exception) in "
              << __FILE__ << " @ line " << __LINE__ << ", what(): "
              << error.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
