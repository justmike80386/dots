#include <iostream>

#include "../xpp.hpp"
#include "../proto/randr.hpp"
#include "../proto/damage.hpp"
#include "../proto/render.hpp"

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h> // XC_cross

namespace x {
  typedef xpp::connection<
                          // xpp::randr::extension,
                          // xpp::damage::extension,
                          // xpp::render::extension
                         >
                            connection;

  typedef xpp::event::registry<connection &
                               // xpp::randr::extension,
                               // xpp::damage::extension,
                               // xpp::render::extension>
                              >
                                 registry;

  typedef xpp::font<connection &> font;
  typedef xpp::cursor<connection &> cursor;
  typedef xpp::window<connection &> window;
  typedef xpp::window<xcb_connection_t *> xcb_window;

  typedef xpp::x::event::key_press<connection &> key_press;
  typedef xpp::x::event::key_release<connection &> key_release;
  typedef xpp::x::event::button_press<connection &> button_press;
  typedef xpp::randr::event::notify<connection &> randr_notify;
  typedef xpp::randr::event::screen_change_notify<connection &> randr_screen_change_notify;
  typedef xpp::damage::event::notify<connection &> damage_notify;
};

int main(int argc, char ** argv)
{
  x::connection connection;
  xcb_font_t font = 0;
  auto cursor_1 = x::cursor(connection, 0);
  auto cursor_2 = x::cursor::create_glyph(connection, font, font,
      XC_cross, XC_cross + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

  return 0;
}
