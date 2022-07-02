#include <iostream>

// #include "../request.hpp"
// #include "../core/connection.hpp"
// #include "../core/window.hpp"
// #include <xcb/xcb.h>
// #include "../gen/sure_dude.hpp"
// #include "../gen/xproto-stub.hpp"


#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h> // XC_cross

#include "../xpp.hpp"
#include "../proto/randr.hpp"
#include "../proto/damage.hpp"
#include "../proto/render.hpp"


/*
namespace xpp { namespace x { namespace error {

void
dispatcher::operator()(const std::shared_ptr<xcb_generic_error_t> &) const
{
  throw std::runtime_error(
        std::string(__PRETTY_FUNCTION__)
      + "\n\thandling errors like a boss with first_error = "
      + std::to_string((uint32_t)m_first_error));
}

}; }; }; // namespace xpp::x::error
*/

// namespace my {
// 
// struct dispatcher {
//   dispatcher(xcb_connection_t *) {}
//   void
//   operator()(const std::shared_ptr<xcb_generic_error_t> &) const
//   {
//     std::cerr << __PRETTY_FUNCTION__ << "\n\thandling errors like a boss" << std::endl;
//   }
// };
// 
// }; // namespace my

/*
namespace xpp {

struct dummy_extension {
  uint8_t first_error = 42;
};

struct connection
  : public dummy_extension
  , public xpp::x::extension
  , public xpp::x::error::dispatcher
  // , public xpp::x::protocol<connection>
  // , virtual public xpp::generic::connection<xpp::connection>
  , public xpp::x::protocol<connection &>
  , virtual public xpp::generic::connection<xpp::connection &>
{
  connection(xcb_connection_t * c)
    : xpp::x::error::dispatcher(
      static_cast<dummy_extension &>(*this).first_error)
    , m_c(c)
    , m_data(new int)
  {
    m_data = std::make_shared<int>();
    std::cerr << "connection c'tor: m_data.use_count(): "
              << m_data.use_count() << std::endl;
  }

  ~connection(void)
  {
    std::cerr << "~connection d'tor: m_data.use_count(): "
              << m_data.use_count() << std::endl;
  }

  connection(connection & other)
    : xpp::x::error::dispatcher(
      static_cast<const dummy_extension &>(other).first_error)
    , m_c(other.m_c)
    // , m_data(other.m_data)
  {
    std::cerr << "connection copy c'tor" << std::endl;
  }

  connection(connection && other) = default;
  // connection(const connection && other)
  //   : xpp::x::error::dispatcher(
  //      static_cast<const dummy_extension &&>(other).first_error)
  //   , m_c(other.m_c)
  // {
  //   std::cerr << "connection move c'tor" << std::endl;
  // }

  operator xcb_connection_t * const(void) const
  {
    return m_c;
  }

  xpp::connection &
  // xpp::connection
  get(void)
  {
    return *this;
  }

  xcb_connection_t * m_c;
  // std::vector<int> m_data;
  std::shared_ptr<int> m_data;
};

class window
  : public xpp::x::window<xpp::connection &>
  // , virtual public xpp::iterable<void>
  , virtual public xpp::iterable<xcb_window_t>
  // , virtual public xpp::xcb::type<const xcb_window_t &>
  // , virtual protected xpp::generic::connection<connection &>
{
  public:
    window(xpp::connection & c, const xcb_window_t & window)
      : m_c(c)
      , m_window(window)
    {}

    // xpp::iterable<const xcb_window_t &>
    virtual
    void
    operator=(xcb_window_t window)
    {
      m_window = window;
    }

    virtual operator const xcb_window_t &(void) const
    {
      return m_window;
    }

  protected:
    xpp::connection & m_c;
    xcb_window_t m_window;

    xpp::connection &
    get(void)
    {
      return m_c;
    }
};

struct dummy_atom {
  dummy_atom(const xcb_atom_t & atom, xcb_connection_t * const) : m_atom(atom) {}
  xcb_atom_t m_atom;
};

}; // xpp
*/

// namespace xpp { namespace generic {
// 
// template<REPLY_TEMPLATE>
// struct reply_getter<xpp::connection, REPLY_SIGNATURE, checked_tag>
// {
//   std::shared_ptr<Reply>
//   operator()(const xpp::connection & c, const Cookie & cookie)
//   {
//     std::cerr << "reply_getter special NO error" << std::endl;
//     return std::shared_ptr<Reply>(ReplyFunction(c, cookie, nullptr), std::free);
//   }
// };
// 
// }; };

bool g_quit = false;

namespace x {
  typedef xpp::connection<xpp::randr::extension,
                          xpp::damage::extension,
                          xpp::render::extension>
                            connection;

  typedef xpp::event::registry<connection &,
                               xpp::randr::extension,
                               xpp::damage::extension,
                               xpp::render::extension>
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

class one_event
  : public xpp::event::sink<x::key_press>
{
  public:
    void handle(const x::key_press &) {}
};

class two_event
  : public xpp::event::sink<x::key_press, x::key_release>
{
  public:
    void handle(const x::key_press &) {}
    void handle(const x::key_release &) {}
};

class more_events
  : public xpp::event::sink<x::randr_notify, x::randr_screen_change_notify, x::damage_notify>
{
  void handle(const x::randr_notify &)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }
  void handle(const x::randr_screen_change_notify &)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }
  void handle(const x::damage_notify &) {}
};

template<typename Connection>
class key_printer
  : public xpp::event::sink<x::key_press,
                            x::key_release,
                            x::button_press>
                            // xpp::randr::event::notify,
                            // xpp::damage::event::notify,
                            // xpp::xfixes::event::selection_notify,
                            // xpp::screensaver::event::notify
                            // >
{
  public:
    template<typename C>
    key_printer(C && c)
      : m_c(std::forward<C>(c))
    {}

    void handle(const x::key_press & e)
    {
      auto kbd_mapping = m_c.get_keyboard_mapping(e->detail, 1);
      auto keysym = *kbd_mapping.keysyms().begin();

      if (keysym == XK_Escape) {
        std::cerr << "quitting" << std::endl;
        m_c.ungrab_keyboard(XCB_TIME_CURRENT_TIME);
        g_quit = true;
      } else {
        std::cerr << "key pressed: " << XKeysymToString(keysym) << std::endl;
      }
    }

    void handle(const x::key_release & e)
    {
      auto kbd_mapping = m_c.get_keyboard_mapping(e->detail, 1);
      auto keysym = *kbd_mapping.keysyms().begin();
      std::cerr << "key released: " << XKeysymToString(keysym) << std::endl;
    }

    void handle(const x::button_press & e)
    {
      m_c.ungrab_pointer(XCB_TIME_CURRENT_TIME);

      std::cerr << "root: 0x"
                << std::hex << e->root << std::dec
                << "; event: 0x"
                << std::hex << e->event << std::dec
                << "; child: 0x"
                << std::hex << e->child << std::dec
                << std::endl;

      // xcb_window_t w = e.event();
      x::window grab_window = e.event<x::window>();
      std::cerr << "grab_window: " << grab_window << std::endl;

      if (e->event == e->root) {
        grab_window = e.child();
        std::cerr << "new grab_window: " << grab_window << std::endl;
        auto translate = grab_window.translate_coordinates(grab_window, 1, 1);
        grab_window = translate->child;
      }

      std::cerr << "grabbing "
                << std::hex << grab_window << std::dec
                << std::endl;

      *m_c.grab_keyboard(true, grab_window,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
      // g_quit = true;
    }

    // void handle(const xpp::randr::event::notify & e)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

    // void handle(const xpp::damage::event::notify & e)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

    // void handle(const xpp::xfixes::event::selection_notify & e)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

    // void handle(const xpp::screensaver::event::notify & e)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

    // does not work
    // template<int OpCode, typename Event>
    // void handle(const xpp::generic::event<OpCode, Event> &)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

  private:
    Connection m_c;
};

xcb_screen_t *
screen_of_display(xcb_connection_t * c, int screen)
{
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator(xcb_get_setup(c));
  for (; iter.rem; --screen, xcb_screen_next(&iter))
    if (screen == 0)
      return iter.data;

  return NULL;
}

int main(int argc, char ** argv)
{
  // int default_screen;
  // xcb_connection_t * c = xcb_connect(nullptr, &default_screen);
  // xcb_screen_t * screen = screen_of_display(c, default_screen);

  // x::error_handler eh;
  x::connection connection;
  // xcb_connection_t * xcb_c = nullptr;

  try {
    x::window window(connection, argc > 1 ? std::strtol(argv[1], NULL, 10) : 0);
    auto tree = window.query_tree();
    std::cerr << "children of window (" << window << "): ";
    for (auto && child : tree.children<x::window>()) {
      std::cerr << child << ", ";
    }
    std::cerr << std::endl;
  } catch (const std::exception & exception) {
    std::cerr << std::endl;
    std::cerr << "window exception: " << exception.what() << std::endl;
  }

  auto net_client_list_stacking_atom = connection.intern_atom(
      false, "_NET_CLIENT_LIST_STACKING");
  auto net_client_list_stacking = connection.get_property(
      false, connection.root(), net_client_list_stacking_atom.atom(),
      XCB_ATOM_WINDOW, 0, UINT32_MAX);

  std::cerr << "_NET_CLIENT_LIST_STACKING (xcb_window_t):" << std::hex;
  for (auto && w : net_client_list_stacking.value<xcb_window_t>()) {
    std::cerr << " 0x" << w;
  }
  std::cerr << std::dec << std::endl;

  std::cerr << "_NET_CLIENT_LIST_STACKING (x::window):";
  for (auto && w : net_client_list_stacking.value<x::window>()) {
    std::cerr << " " << w;
  }
  std::cerr << std::endl;

  std::cerr << "_NET_CLIENT_LIST_STACKING (x::xcb_window):";
  for (auto && w : net_client_list_stacking.value<x::xcb_window>()) {
    std::cerr << " " << w;
  }
  std::cerr << std::endl;

// #ifndef __clang__
  // static_cast<xpp::randr::protocol<const x::connection &> &>(connection)
  //   .query_version(XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);

  // connection.interface<xpp::randr::extension>()
  connection.randr().query_version(
      XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);

  const xpp::randr::extension & randr = connection.extension<xpp::randr::extension>();
  // xpp::randr::extension & randr = static_cast<xpp::randr::extension &>(connection);
  std::cerr << "RandR Extension" << std::endl;
  std::cerr << "\tfirst_event: " << (int)randr->first_event << std::endl;
  std::cerr << "\tfirst_error: " << (int)randr->first_error << std::endl;

  connection.select_input_checked(connection.root(), XCB_RANDR_NOTIFY);

  const xpp::damage::extension & damage = connection.extension<xpp::damage::extension>();
  std::cerr << "Damage Extension" << std::endl;
  std::cerr << "\tfirst_event: " << (int)damage->first_event << std::endl;
  std::cerr << "\tfirst_error: " << (int)damage->first_error << std::endl;
// #endif

  x::registry registry(connection);

  // key_printer<x::connection &> key_printer(connection);
  std::vector<key_printer<x::connection &>> key_printers(100, connection);
  // std::vector<key_printer<x::connection &> *> key_printers(100, new key_printer<x::connection &>(connection));

  const int n = 2;
  // registry.attach(0, &key_printer);
  for (int i =  0; i < n; ++i) {
    registry.attach(0, &key_printers[i]);
    // registry.attach(0, key_printers[i]);
  }

  for (int i =  0; i < n - 1; ++i) {
    registry.detach(0, &key_printers[i]);
    // registry.detach(0, key_printers[i]);
  }

  one_event oe;
  two_event te;
  more_events me;

  registry.attach(0, &oe);
  registry.attach(0, &te);
  registry.detach(0, &oe);
  registry.detach(0, &te);

  registry.attach(0, &me);
  // registry.detach(0, &me);


  // auto font = connection.generate_id();
  // connection.open_font(font, "cursor");

  // auto cursor = connection.generate_id();
  // connection.create_glyph_cursor(cursor, font, font,
      // XC_cross, XC_cross + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);
  // connection.close_font(font);
  // connection.close_cursor(cursor);
    // x::cursor cursor_xid = x::cursor(connection, 0);


  try {
    x::font font = x::font::open_checked(connection, "cursor");

    x::cursor cursor = x::cursor::create_glyph_checked(connection, font, font,
        XC_cross, XC_cross + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);

    *connection.grab_pointer(false, connection.root(),
                             XCB_EVENT_MASK_BUTTON_PRESS,
                             XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                             XCB_NONE, cursor, XCB_TIME_CURRENT_TIME);

    std::cerr << "Please click on a window" << std::endl;

  } catch (const std::exception & error) {
    std::cerr << "Exception (std::exception) in "
              << __FILE__ << " @ line " << __LINE__ << ", what(): "
              << error.what() << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "Exception (std::shared_ptr<xcb_generic_error_t>) in "
              << __FILE__ << " @ line " << __LINE__ << ", error_code: "
              << (int)error->error_code << std::endl;
    std::exit(EXIT_FAILURE);
  } catch (...) {
    std::cerr << "Exception (...) in "
              << __FILE__ << " @ line " << __LINE__
              << std::endl;
    std::exit(EXIT_FAILURE);
  }


  // xcb_randr_get_output_info_cookie_t goic = xcb_randr_get_output_info_unchecked(connection, -1, XCB_TIME_CURRENT_TIME);
  // xcb_randr_get_output_info_reply_t * goir = xcb_randr_get_output_info_reply(connection, goic, nullptr);

  // XCB_RANDR_BAD_OUTPUT
  auto output_info = connection.get_output_info_unchecked(-1);
  output_info.get();

  // XCB_RANDR_BAD_CRTC
  // auto crtc_info = connection.get_crtc_info_unchecked(-1);
  // crtc_info.get();

  // auto output_property = connection.query_output_property_unchecked(-1, -1);
  // output_property.get();

  connection.change_output_property(-1, -1, -1, 0, 0, 0, nullptr);

  // connection.map_window(-1);

  // auto & damage_proto = static_cast<xpp::damage::protocol<const x::connection &> &>(connection);
  // damage_proto.create(-1, -1, 0);
  // damage_proto.destroy(-1);
  // damage_proto.subtract(-1, 0, 0);

  connection.query_pict_index_values_unchecked(-1);
  connection.change_picture(-1, 0, nullptr);

// clang_complete does not like this
// causes vim to segfault
// #if not defined __clang__

  while (! g_quit) {
    connection.flush();
    try {
      registry.dispatch(connection.wait_for_event());
    } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
      std::cerr << "Caught std::shared_ptr<xcb_generic_error_t> in event loop "
                << "(code: " << (int)error->error_code << ")" << std::endl;
    } catch (const std::exception & error) {
      std::cerr << "Caught std::exception in event loop: " << error.what() << std::endl;
    } catch (...) {
      std::cerr << "Something really bad has happened" << std::endl;
    }
  }

// #endif

  // xpp::x::event::key_press<x::connection &>
  //   xek_1(connection, std::shared_ptr<xcb_generic_event_t>((xcb_generic_event_t *)new xcb_key_press_event_t));
  // xpp::x::event::key_press<xcb_connection_t *>
  //   xek_2(connection, std::shared_ptr<xcb_generic_event_t>(((xcb_generic_event_t *)new xcb_key_press_event_t)));
  // xpp::x::event::key_press<xcb_connection_t *>
  //   xek_3(xcb_c, std::shared_ptr<xcb_generic_event_t>(((xcb_generic_event_t *)new xcb_key_press_event_t)));

  // xpp::window<x::connection>      w_1(connection);
  // xpp::window<xcb_connection_t *> w_2(xcb_c);
  // static_cast<xcb_window_t &>(w_1) = 12;

  // if (xcb_connection_has_error(c)) {
  //   std::cerr << "Connection has error" << std::endl;
  //   return EXIT_FAILURE;
  // }

  // xpp::x::error::dispatcher ed(connection);

  // xcb_void_cookie_t mwc = xcb_map_window_checked(c, -1);
  // xcb_flush(c);
  // xcb_generic_error_t * error = xcb_request_check(c, mwc);
  // xcb_flush(c);
  // if (error) {
  //   std::free(error);
  //   std::cerr << "mwc error" << std::endl;
  // }

  /*
  std::shared_ptr<xcb_query_tree_reply_t> qtr;

  typedef xpp::fixed::detail::simple<
      x::connection &, xcb_window_t,
      SIGNATURE(xcb_query_tree_children),
      SIGNATURE(xcb_query_tree_children_length)>
        simple_query_tree_iterator;
  simple_query_tree_iterator(connection, qtr, 0);

  typedef xpp::fixed::detail::object<
      x::connection &, x::window,
      SIGNATURE(xcb_query_tree_children),
      SIGNATURE(xcb_query_tree_children_length)>
        object_query_tree_iterator;
  object_query_tree_iterator(connection, qtr, 0);
  */

  // typedef xpp::iterator<
  //     x::connection, x::window,
  //     SIGNATURE(xcb_query_tree_children),
  //     SIGNATURE(xcb_query_tree_children_length)>
  //       query_tree_iter;

  // auto qt_iter = query_tree_iter(connection, qtr, 0);

  // typedef xpp::generic::list<x::connection,
  //                            xcb_query_tree_reply_t,
  //                            query_tree_iter>
  //                              query_tree_list;

  // auto qt_list = query_tree_list(connection, qtr);
  // auto qt_list_begin = qt_list.begin();

  // std::shared_ptr<xcb_query_tree_reply_t> qtr;
  // auto simple_iter = xpp::fixed::iterator::simple<
  //     x::connection, xcb_window_t, xcb_window_t, xcb_query_tree_reply_t,
  //     xcb_query_tree_children,
  //     xcb_query_tree_children_length>(connection, qtr, 0);

  // auto object_iter = xpp::fixed::iterator::object<
  //     x::connection, xcb_window_t, x::window, xcb_query_tree_reply_t,
  //     xcb_query_tree_children,
  //     xcb_query_tree_children_length>(connection, qtr, 0);

  // std::shared_ptr<xcb_get_font_path_reply_t> fpr;
  // typedef xpp::iterator<x::connection,
  //                       xcb_str_t,
  //                       xcb_str_t,
  //                       SIGNATURE(xcb_str_next),
  //                       SIGNATURE(xcb_str_sizeof),
  //                       SIGNATURE(xcb_get_font_path_path_iterator)>
  //                         font_path_iter;

  std::cerr << "fonts:" << std::endl;
  auto fonts = connection.list_fonts(8, 1, "*");
  for (auto && name : fonts.names()) {
    std::cerr << "font [" << name.length() << "]: " << name << std::endl;
  }

  std::cerr << "paths:" << std::endl;
  // auto font_paths = connection.get_font_path();
  for (auto && path : connection.get_font_path().path()) {
    std::cerr << "path [" << path.length() << "]: " << path << std::endl;
  }

  auto tree = connection.root<x::window>().query_tree();
  std::cerr << "children: ";
  for (auto && child : tree.children<x::xcb_window>()) {
    std::cerr << child << ", ";
    auto siblings = child.query_tree();
    std::cerr << "(siblings: ";
    for (auto && sibling : siblings.children()) {
      std::cerr << sibling << ", ";
    }
    std::cerr << "), " << std::endl;
  }
  std::cerr << std::endl;

  // auto tree_2 = connection.query_tree(screen->root);
  // std::cerr << "children: ";
  // for (auto & w : tree_2.children<x::window>()) {
  //   std::cerr << w << ", ";
  // }
  // std::cerr << std::endl;

  // xcb_window_t window = 27263111;
  xcb_window_t window = 0;

  // test::map_window(c, window);
  // test::map_window(c, window, 0);
  xpp::x::map_window(connection, window);
  connection.map_window(window);

  try {
    // test::map_window_checked(c, window);
    // xpp::x::map_window_checked(c, window);
    connection.map_window_checked(window);
  } catch (const std::exception & e) {
    std::cerr << "CATCH: map_window exception:" << std::endl
              << "\twhat(): " << e.what() << std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "CATCH: map_window error: " << (int)error->error_code << std::endl;
  }

  // auto attrs_0 = test::get_window_attributes(c, window);
  // auto attrs_1 = test::get_window_attributes(connection, window);
  // auto attrs_2 = test::get_window_attributes_unchecked(connection, window);

  xpp::x::reply::checked::get_window_attributes<xcb_connection_t *>
    gwar(connection, window);

  auto attrs_0 = connection.get_window_attributes(window);
  // auto attrs_0 = xpp::x::get_window_attributes(c, window);
  auto attrs_1 = xpp::x::get_window_attributes(connection, window);
  auto attrs_2 = xpp::x::get_window_attributes_unchecked(connection, window);

  try {
    attrs_0.get();
  } catch (const std::exception & e) {
    std::cerr << "CATCH: attrs_0 exception:" << std::endl
              << "\twhat(): " << e.what() << std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "CATCH: attrs_0 std::shared_ptr<xcb_generic_error_t>::error_code: " << (int)error->error_code << std::endl;
  }

  if (! attrs_0) {
    std::cerr << "attrs_0 invalid" << std::endl;
  } else {
    std::cerr << "attrs_0 valid" << std::endl;
  }

  try {
    attrs_1.get();
  } catch (const std::exception & e) {
    std::cerr << "CATCH: attrs_1 exception:" << std::endl
              << "\twhat(): " << e.what() << std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "CATCH: attrs_1 std::shared_ptr<xcb_generic_error_t>::error_code: " << (int)error->error_code << std::endl;
  }

  if (! attrs_1) {
    std::cerr << "attrs_1 invalid" << std::endl;
  } else {
    std::cerr << "attrs_1 valid" << std::endl;
  }

  try {
    attrs_2.get();
  } catch (const std::exception & e) {
    std::cerr << "CATCH: attrs_2 exception:" << std::endl
              << "\twhat(): " << e.what() << std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "CATCH: attrs_2 std::shared_ptr<xcb_generic_error_t>::error_code: " << (int)error->error_code << std::endl;
  }

  if (! attrs_2) {
    std::cerr << "attrs_2 invalid" << std::endl;
  } else {
    std::cerr << "attrs_2 valid" << std::endl;
  }

  try {
    auto my_string_atom = xpp::x::intern_atom(connection, false, "MY_STRING");
    std::string my_string("SUPER COOL");

    std::cerr << "atom \"MY_STRING\": " << my_string_atom.atom() << std::endl;;

    auto atom_name = connection.get_atom_name(my_string_atom.atom());
    std::cerr << "atom name: " << atom_name.name() << std::endl;;

    // xpp::x::change_property_checked(c,
    connection.change_property_checked(
        XCB_PROP_MODE_REPLACE, connection.root(),
        my_string_atom.atom(), XCB_ATOM_STRING, 8,
        my_string.begin(), my_string.end());

    // xpp::x::change_property_checked(c,
    connection.change_property_checked(
        XCB_PROP_MODE_REPLACE, 0,
        my_string_atom.atom(), XCB_ATOM_STRING, 8,
        my_string.length(), my_string.c_str());
  } catch (const std::exception & e) {
    std::cerr << "change property failed: " << e.what() << std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr << "generic error: " << (int)error->error_code << std::endl;
  }

  connection.flush();

  // xcb_disconnect(c);


  /*
  std::cerr << std::endl << "intern_atom 1" << std::endl;
  auto atom_reply_1 =
    xpp::x::intern_atom_unchecked(c, false, 25, "_NET_CLIENT_LIST_STACKING");
  atom_reply_1.get();
  if (atom_reply_1) {
    std::cerr<<"atom_reply_1 success"<<std::endl;
  }

  std::cerr << std::endl << "intern_atom 2" << std::endl;
  auto atom_reply_2 = xpp::x::intern_atom(c, false, "_NET_CLIENT_LIST_STACKING");
  atom_reply_2.get();
  if (atom_reply_2) {
    std::cerr<<"atom_reply_2 success"<<std::endl;
  }

  std::cerr << std::endl << "intern_atom 3" << std::endl;
  auto atom_reply_3 = xpp::x::intern_atom(c, false, "_NET_CLIENT_LIST_STAC");
  try {
    atom_reply_3.get();
    std::cerr<<"atom_reply_3 success"<<std::endl;
  } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
    std::cerr<<"CATCH: atom_reply_3 success, error_code: "
    << (int)error->error_code <<std::endl;
  }

  std::cerr << std::endl << "intern_atom 1 reply: ";
  xcb_atom_t atom = atom_reply_1->atom;
  std::cerr << (int)atom << std::endl;

  std::cerr << std::endl << "intern_atom 2 reply: ";
  auto dummy_atom = atom_reply_2.atom<xpp::dummy_atom>();
  std::cerr << (int)dummy_atom.m_atom << std::endl;

  auto my_string_atom = xpp::x::intern_atom(c, true, "MY_STRING");
  std::string my_string("LADIDA");
  xpp::x::change_property(
      c, XCB_PROP_MODE_REPLACE, 0, my_string_atom.atom(), XCB_ATOM_STRING, 8,
      my_string.begin(), my_string.end());
  */

  /*
  std::cerr << std::endl << "intern_atom 1" << std::endl;
  auto atom_reply_1 = intern_atom(c, false, 25, "_NET_CLIENT_LIST_STACKING");

  std::cerr << std::endl << "intern_atom 2" << std::endl;
  auto atom_reply_2 = intern_atom(c, false, std::string("_NET_CLIENT_LIST_STACKING"));

  std::cerr << std::endl << "intern_atom 3" << std::endl;
  auto atom_reply_3 = intern_atom<xpp::connection,
                                  my::dispatcher,
                                  xpp::x::error::dispatcher>
                                    (c, false, "FOO BAZ");
  atom_reply_3.get();

  std::cerr << std::endl << "intern_atom 1 reply: ";
  xcb_atom_t atom = atom_reply_1->atom;
  std::cerr << (int)atom << std::endl;

  std::cerr << std::endl << "intern_atom 2 reply: ";
  auto dummy_atom = atom_reply_2.atom<xpp::dummy_atom>();
  std::cerr << (int)dummy_atom.m_atom << std::endl;

  std::cerr << std::endl << "map_window 1" << std::endl;
  map_window(connection, 0);
  std::cerr << std::endl << "map_window 2" << std::endl;
  map_window<xpp::connection>(c, 0);
  std::cerr << std::endl << "map_window 3" << std::endl;
  map_window<xpp::connection, my::dispatcher, xpp::x::error::dispatcher>(connection, 0);

  auto my_string_atom =
    intern_atom<xpp::connection, my::dispatcher>(connection, true, "MY_STRING");
  std::string my_string("LADIDA");
  change_property<xpp::connection, my::dispatcher>(
      c, XCB_PROP_MODE_REPLACE, 0, my_string_atom.atom(), XCB_ATOM_STRING, 8,
      my_string.begin(), my_string.end());
  */

  /*
  std::cerr << std::endl << "auto wa_1" << std::endl;
  auto wa_1 = get_window_attributes(c, 0);
  std::cerr << std::endl << "auto wa_2" << std::endl;
  auto wa_2 = get_window_attributes<xpp::connection,
                                    my::dispatcher,
                                    xpp::x::error::dispatcher>
                                      (connection, 0);
  std::cerr << std::endl << "auto wa_3" << std::endl;
  auto wa_3 = get_window_attributes<xpp::connection,
                                    my::dispatcher,
                                    xpp::x::error::dispatcher>
                                      (connection, 0);

  std::cerr << std::endl << "wa_1" << std::endl;
  auto r1 = wa_1.get();
  std::cerr << std::endl << "wa_2" << std::endl;
  auto r2 = wa_2.get();
  std::cerr << std::endl << "wa_3" << std::endl;
  auto r3 = wa_3.get();
  */

  // try {
  //   std::cerr << "YOLOLOLOL" << std::endl;
  //   xcb_colormap_t cm = wa_3.colormap();
  //   cm = wa_1->colormap;
  //   cm = r1->colormap;
  // } catch (...) {}

  // map_window<xpp::connection>(c, 0);
  // map_window<xpp::connection, my::dispatcher, xpp::x::error::dispatcher>(c, 0);

  // auto wa_cookie = get_window_attributes_cookie(c, 0);
  // auto wa_reply_1 = get_window_attributes_reply<xpp::connection>(connection, wa_cookie);
  // auto wa_reply_2 = get_window_attributes_reply<
  //   xpp::connection, my::dispatcher, xpp::x::error::dispatcher>(
  //     connection, get_window_attributes_cookie()(c, 0));

  // wa_reply_1.get();
  // wa_reply_2.get();

  // auto wa = get_window_attributes(c, 0);
  // auto wa_error = get_window_attributes(ed, c, 0);
  // wa_error.get();

  // map_window_cookie<xpp::connection>()(connection, 0);
  // map_window_cookie<xpp::connection>()(ed, connection, 0);
  // map_window_cookie<xpp::connection>()(connection, connection, 0);
  // map_window_cookie<xpp::connection>()(xpp::x::error::dispatcher(), connection, 0);

  // get_window_attributes_cookie wac_1(c, 0);
  // get_window_attributes_cookie wac_2(c, 0);

  // get_window_attributes_reply<> war_1(connection, wac_1);
  // get_window_attributes_reply<> war_2(connection, wac_2);

  // get_window_attributes_reply<my::dispatcher, xpp::x::error::dispatcher>
  //   war_3(connection, get_window_attributes_cookie()(c, 0), my::dispatcher(), xpp::x::error::dispatcher());

  // war_1.get();
  // war_2.get();
  // war_3.get();


  // map_window_cookie<xpp::connection> mwc_unchecked;
  // mwc_checked(, connection, 0);
  // mwc_checked(xpp::x::error::dispatcher(), connection, 0);
  // mwc_unchecked(connection, 0);
  // map_window_cookie<checked,   xpp::connection> mwc_checked;
  // map_window_cookie<unchecked, xpp::connection> mwc_unchecked;

  // map_window(c, 0);
  // map_window<xpp::generic::checked>(c, 0);
  // auto reply = get_window_attributes(connection, 0);
  // reply.get();

  return EXIT_SUCCESS;
}
