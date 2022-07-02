#include <climits>
#include <unistd.h>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/extensions/Xrandr.h>
#include <X11/cursorfont.h> // XC_cross

// #include <xcb/randr.h>
// // #include "../gen/xproto_requests_with_accessors.hpp"
// #include "../gen/randr_requests.hpp"
// #include "../gen/xproto.hpp"
// #include "../gen/randr.hpp"

// #include "../xpp.hpp"

#include "../event.hpp"
#include "../core/connection.hpp"
// #include "../gen/xproto.hpp"
// #include "../gen/randr.hpp"

bool g_quit = false;

class key_printer
  : public xpp::event::sink<xpp::x::event::key_press,
                            xpp::x::event::key_release,
                            xpp::x::event::button_press,
                            xpp::randr::event::notify,
                            xpp::damage::event::notify,
                            xpp::xfixes::event::selection_notify,
                            xpp::screensaver::event::notify
                            >
{
  public:
    key_printer(const xpp::x::protocol & x)
      : m_x(x)
    {}

    void handle(const xpp::x::event::key_press & e)
    {
      auto kbd_mapping = m_x.get_keyboard_mapping(e->detail, 1);
      auto keysym = *kbd_mapping.keysyms().begin();

      if (keysym == XK_Escape) {
        std::cerr << "quitting" << std::endl;
        m_x.ungrab_keyboard();
        g_quit = true;
      } else {
        std::cerr << "key pressed: " << XKeysymToString(keysym) << std::endl;
      }
    }

    void handle(const xpp::x::event::key_release & e)
    {
      auto kbd_mapping = m_x.get_keyboard_mapping(e->detail, 1);
      auto keysym = *kbd_mapping.keysyms().begin();
      std::cerr << "key released: " << XKeysymToString(keysym) << std::endl;
    }

    void handle(const xpp::x::event::button_press & e)
    {
      m_x.ungrab_pointer();

      std::cerr << "root: 0x"
                << std::hex << e->root << std::dec
                << "; event: 0x"
                << std::hex << e->event << std::dec
                << "; child: 0x"
                << std::hex << e->child << std::dec
                << std::endl;

      xpp::window grab_window = e.event<xpp::window>();
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

      *m_x.grab_keyboard(true, grab_window,
                         XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
    }

    void handle(const xpp::randr::event::notify & e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    void handle(const xpp::damage::event::notify & e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    void handle(const xpp::xfixes::event::selection_notify & e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    void handle(const xpp::screensaver::event::notify & e)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    // does not work
    // template<int OpCode, typename Event>
    // void handle(const xpp::generic::event<OpCode, Event> &)
    // {
    //   std::cerr << __PRETTY_FUNCTION__ << std::endl;
    // }

  private:
    const xpp::x::protocol & m_x;
};

class xevent : public xpp::x::event::dispatcher {
  public:

    template<typename Event>
    void operator()(const Event &) const
    {
    }

    // template<int OpCode, typename Event>
    // void operator()(const xpp::generic::event<OpCode, Event> &) const
    // {
    // }

    void doit(xcb_generic_event_t * const e) const
    {
#if not defined __clang__
      dispatcher::operator()(*this, e);
#endif
    }

  protected:
    operator xcb_connection_t * const(void) { return nullptr; }
};

int main(int argc, char ** argv)
{
  xpp::connection<xpp::extension::randr,
                  xpp::extension::render,
                  xpp::extension::xv,
                  xpp::extension::damage,
                  xpp::extension::xinerama,
                  xpp::extension::xfixes,
                  xpp::extension::input,
                  xpp::extension::screensaver>
                    connection("");

  // xpp::request::x::map_window(connection, 0);
  // xpp::request::x::query_tree(connection, 0);

  xpp::event::registry<xpp::extension::randr,
                       xpp::extension::render,
                       xpp::extension::xv,
                       xpp::extension::damage,
                       xpp::extension::xinerama,
                       xpp::extension::xfixes,
                       xpp::extension::input,
                       xpp::extension::screensaver>
                         registry(connection);

  std::vector<key_printer *> printers(100, new key_printer(connection));

  for (int i =  0; i < 100; ++i) {
    registry.attach(0, printers[i]);
  }

  for (int i =  0; i < 99; ++i) {
    registry.detach(0, printers[i]);
  }

  auto font_id = connection.generate_id();
  connection.open_font(font_id, "cursor");
  auto cursor_id = connection.generate_id();
  connection.create_glyph_cursor(cursor_id, font_id, font_id,
      XC_cross, XC_cross + 1, 0, 0, 0, 0xffff, 0xffff, 0xffff);
  connection.close_font(font_id);

  *connection.grab_pointer(false, connection.root(),
                           XCB_EVENT_MASK_BUTTON_PRESS,
                           XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                           XCB_NONE, cursor_id);

  connection.free_cursor(cursor_id);

  std::cerr << "Please click on a window" << std::endl;

// clang_complete does not like this
// causes vim to segfault
#if not defined __clang__

  while (! g_quit) {
    connection.flush();
    registry.dispatch(connection.wait_for_event());
  }

#endif


  // WATCHOUT FOR THE RETURN
  // >>>>>>>>>>>>>>>>>>>>>>>
  return 0;
  // <<<<<<<<<<<<<<<<<<<<<<<





  *((xpp::randr::protocol &)connection).query_version(
      XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);

  // auto & randr = (xpp::protocol::randr &)c;
  // auto version = randr.query_version(
  //     XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);
  // std::cerr << "randr_major: " << version->major_version
  //           << "; randr_minor: " << version->minor_version << std::endl;

  // auto screens = connection.query_screens();

  // for (auto & info : screens.screen_info()) {
  //   std::cerr << "screen @ "
  //             << info.x_org << "x" << info.x_org
  //             << "+"
  //             << info.width << "+" << info.height
  //             << std::endl;
  // }

  std::cerr << "connection.root().query_tree().children<xcb_window_t>():" << std::endl;
  for (auto & window : connection.root().query_tree().children()) {
    std::cerr << window << ", ";
  }
  std::cerr << std::endl;

  std::cerr << "connection.root().query_tree().children<xpp::window>():" << std::endl;
  for (auto & window : connection.root().query_tree().children<xpp::window>()) {
    std::cerr << window << ": ";
    for (auto & child : window.query_tree().children<xpp::window>()) {
      std::cerr << child << ", ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::endl;

  auto atom = xpp::request::x::intern_atom(
      connection, false, "_NET_CLIENT_LIST_STACKING");
  auto property = xpp::request::x::get_property(
      connection, false, connection.root(), atom->atom, XCB_ATOM_WINDOW, 0, UINT_MAX);

  std::cerr << "windows (xcb_window_t):" << std::hex << std::endl;
  for (auto & w : property.value<xcb_window_t>()) {
    std::cerr << "0x" << w << ", ";
  }
  std::cerr << std::dec << std::endl;

  std::cerr << "windows (xpp::window)" << std::hex << std::endl;
  for (auto & w : property.value<xpp::window>()) {
    std::cerr << w << ": ";
    for (auto & child : w.query_tree().children<xpp::window>()) {
      std::cerr << child << ", ";
    }
    std::cerr << std::endl;
  }
  std::cerr << std::dec << std::endl;

  std::cerr << "hosts:" << std::endl;
  auto hosts = xpp::request::x::list_hosts(connection);
  for (auto & host : hosts.hosts()) {
    std::cerr << "host: " << xcb_host_address(&host) << std::endl;
  }

  std::cerr << "fonts:" << std::endl;
  auto fonts = xpp::request::x::list_fonts(connection, 8, 1, "*");
  for (auto & name : fonts.names()) {
    std::cerr << "font (" << name.length() << "): " << name << std::endl;
  }

  auto font_paths = xpp::request::x::get_font_path(connection);
  for (auto & path : font_paths.path()) {
    std::cerr << "path (" << path.length() << "): " << path << std::endl;
  }

  auto screen_info = xpp::request::randr::get_screen_info(connection, connection.root());
  std::cerr << "nsizes: " << (int)screen_info->nSizes << std::endl;
  std::cerr << "SizeID: " << (int)screen_info->sizeID << std::endl;
  int i = 0;
  for (auto & rate : screen_info.rates()) {
    if (++i > screen_info->nSizes) break;

    uint16_t * rates = xcb_randr_refresh_rates_rates(&rate);
    std::cerr << "rates (length: "
      << xcb_randr_refresh_rates_rates_length(&rate)
      << "): ";

    for (int j = 0; j < xcb_randr_refresh_rates_rates_length(&rate); ++j) {
      std::cerr << rates[j];
      if (j < xcb_randr_refresh_rates_rates_length(&rate) - 1) {
        std::cerr << ", ";
      }
    }
    std::cerr << std::endl;

  }

/*
  xcb_list_hosts_cookie_t hosts_cookie = xcb_list_hosts(c);
  xcb_list_hosts_reply_t * hosts_reply = xcb_list_hosts_reply(connection, hosts_cookie, NULL);

  std::cerr << "list hosts" << std::endl;
  xcb_host_iterator_t host_iter = xcb_list_hosts_hosts_iterator(hosts_reply);
  for ( ; host_iter.rem > 0; xcb_host_next(&host_iter) ) {
    xcb_host_t * host = (xcb_host_t *)host_iter.data;
    std::cerr << "host adress: " << xcb_host_address(host) << std::endl;
    std::cerr << "host family: " << (int)host->family << std::endl;
    std::cerr << "host address_len: " << (int)host->address_len << std::endl;
  }

  std::cerr << "Randr refresh rates: " << std::endl;
  xcb_randr_get_screen_info_cookie_t screen_info_cookie =
    xcb_randr_get_screen_info(connection, connection.root());
  xcb_randr_get_screen_info_reply_t * screen_info_reply =
    xcb_randr_get_screen_info_reply(connection, screen_info_cookie, NULL);
  std::cerr << "Randr refresh rates length: "
            << xcb_randr_get_screen_info_rates_length(screen_info_reply)
            << std::endl;

  xcb_randr_refresh_rates_iterator_t refresh_rates_iter =
    xcb_randr_get_screen_info_rates_iterator(screen_info_reply);

  for (int i = 0; i < screen_info->nSizes; ++i) {
    xcb_randr_refresh_rates_next(&refresh_rates_iter);

    xcb_randr_refresh_rates_t * rate =
      (xcb_randr_refresh_rates_t *)refresh_rates_iter.data;
    uint16_t * rates = xcb_randr_refresh_rates_rates(rate);
    std::cerr << "rates (length: "
      << xcb_randr_refresh_rates_rates_length(rate)
      << "): ";
    for (int j = 0; j < xcb_randr_refresh_rates_rates_length(rate); ++j) {
      std::cerr << rates[j];
      if (j < xcb_randr_refresh_rates_rates_length(rate) - 1) {
        std::cerr << ", ";
      }
    }
    std::cerr << std::endl;
  }

  auto pict_formats = xpp::request::render::query_pict_formats(c);
  for (auto & format : pict_formats.formats()) {
    std::cerr << "format.depth: " << (int)format.depth << std::endl;;
  }
  */

    /*
  {
  Display * dpy = XOpenDisplay(NULL);
  Window root = DefaultRootWindow(dpy);
  XRRScreenConfiguration * screen_cfg = XRRGetScreenInfo(dpy, root);
  std::cerr << "rate: " << XRRConfigCurrentRate(screen_cfg) << std::endl;
  int nrates = 0;
  short * rates = XRRConfigRates(screen_cfg, 10, &nrates);
  std::cerr << "nrates: " << nrates << std::endl;
  for (int i = 0; i < nrates; ++i) {
    std::cerr << "rate: " << rates[i] << std::endl;
  }
  }
  */

// auto iter = xpp::iterator<xcb_str_t, xcb_str_t, xcb_list_fonts_reply_t,
//      xcb_str_iterator_t,
//                xpp::callable<decltype(xcb_str_next), xcb_str_next>,
//                xpp::callable<decltype(xcb_str_sizeof), xcb_str_sizeof>,
//                xpp::callable<decltype(xpp::callable_test), xpp::callable_test>>();

// auto iter2 = xpp::iterator<int, int, int, int,
//                xpp::callable<decltype(xpp::next), xpp::next>,
//                xpp::callable<decltype(xpp::size_of), xpp::size_of>,
//                xpp::callable<decltype(xpp::get_iterator), xpp::get_iterator>>();

  // xcb_window_t root = 0;
  // auto x = connection.x();
  // auto windows = connection.x().query_tree(root);
  // auto primary_output = connection.randr().get_output_primary(root);

  // auto window = *windows.children().begin();
  // auto geometry = window.get_geometry();



  // primary_output->output;

  /*
  xpp::connection c("");
  // xpp::xproto::connection cc(*c);
  std::cerr << "c: " << *c << std::endl;
  std::cerr << "connection.root() " << connection.root() << std::endl;

  uint32_t mask = XCB_CW_BACK_PIXEL
                | XCB_CW_BORDER_PIXEL
                | XCB_CW_OVERRIDE_REDIRECT;
  uint32_t values[] = { 0xff123456, 0xff654321, true };
  // uint32_t values[] = { 0, 0, true };

  xpp::window window_1(
      connection, 24, *connection.root(), 25, 25, 50, 50, 2,
      XCB_WINDOW_CLASS_INPUT_OUTPUT, XCB_WINDOW_CLASS_COPY_FROM_PARENT,
      mask, values);

  std::cerr << window_1 << std::endl;
  */

  // {
  //   auto window_2 = window_1;
  //   std::cerr << window_2 << std::endl;
  //   {
  //     auto window_3 = window_2;
  //     std::cerr << window_3 << std::endl;
  //     window_3.map();
  //     connection.flush();
  //   }
  //   sleep(3);
  // }

  // window_1.map();
  // connection.flush();
  // sleep(1);

  // connection.root().query_tree().children();


  // intern_atom
  /*
  auto atom =
    xpp::xproto::request::intern_atom(*connection, false, "_NET_CLIENT_LIST_STACKING");
  auto atom_name = xpp::xproto::request::get_atom_name(*connection, atom->atom);
  std::cerr << atom_name.name() << std::endl;

  auto property = xpp::xproto::request::get_property(
      *connection, false, *connection.root(), atom->atom, XCB_ATOM_WINDOW, 0, UINT_MAX);

  std::cerr << "windows" << std::hex << std::endl;
  for (auto & p : property.value<xcb_window_t>()) {
    std::cerr << "0x" << p << ", ";
  }
  std::cerr << std::dec << std::endl;
  */

  /*
  typedef xpp::generic::fixed_size::iterator<void,
                                             xcb_window_t,
                                             xcb_get_property_reply_t,
                                             xcb_get_property_value,
                                             xcb_get_property_value_length>
                                               window_iterator;

  window_iterator window_begin = property.value<xcb_window_t>().begin();
  window_iterator window_end = property.value<xcb_window_t>().end();
  */

  // list_fonts_with_info && list_fonts
  /*
  auto fonts_with_info =
    xpp::xproto::request::list_fonts_with_info(*connection, 8, "*");

  for (int i = 0; i < 8; ++i) {
    std::cerr << fonts_with_info.name() << std::endl;
    fonts_with_info.reset();
  }

  auto fonts = xpp::xproto::request::list_fonts(*connection, 8, "*");
  for (auto & font : fonts.names()) {
    std::cerr << "(" << font.length() << "): " << font << std::endl;
  }
  */

  // auto grab = xpp::xproto::request::grab_keyboard(
  //     *connection, false, connection.root(), XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

  // query_tree iterator
  /*
  typedef xpp::generic::fixed_size::iterator<xcb_window_t,
                                             xcb_window_t,
                                             xcb_query_tree_reply_t,
                                             xcb_query_tree_children,
                                             xcb_query_tree_children_length>
                                               children_iterator;

  children_iterator children_begin, children_end;
  */

  // {
  //   auto tree = xpp::xproto::request::query_tree(*connection, connection.root());
  //   children_begin = tree.children().begin();
  //   children_end = tree.children().end();
  // }

  // std::cerr << "windows" << std::endl;
  // for (auto it = children_begin; it != children_end; ++it) {
  //   std::cerr << std::hex << "0x" << *it << ", ";
  // }
  // std::cerr << std::endl;

  // std::cerr << "crtcs" << std::endl;
  // auto primary = xpp::request::randr::get_output_primary(*connection, connection.root());
  // for (auto & crtc : xpp::request::randr::get_output_info(
  //       *connection, primary->output, XCB_TIME_CURRENT_TIME).crtcs()) {
  //   std::cerr << "crtc: " << crtc << std::endl;
  // }

  // list_hosts
  /*
  std::cerr << "hosts" << std::endl;
  for (auto & host : xpp::xproto::request::list_hosts(*c).hosts()) {
    std::cerr << std::string((char *)xcb_host_address(&host),
                             xcb_host_address_length(&host))
              << std::endl;
  }
  */

  // std::cerr << "extensions" << std::endl;
  // for (auto & name : xpp::xproto::request::list_extensions(*c).names()) {
  //   std::cerr << std::string((char *)xcb_str_name(&name), xcb_str_name_length(&name)) << std::endl;
  // }

  // get_font_path
  /*
  for (auto & path : xpp::xproto::request::get_font_path(*c).path()) {
    std::cerr << "(" << path.length() << ") path: " << path << std::endl;
  }
  */

  // list_fonts
  /*
  const size_t n_fonts = 16;

  std::cerr << "fonts (xpp 2)" << std::endl;

  typedef xpp::generic::variable_size::iterator<xcb_str_t,
                                                xcb_str_t,
                                                xcb_list_fonts_reply_t,
                                                xcb_str_iterator_t,
                                                xcb_str_next,
                                                xcb_str_sizeof,
                                                xcb_list_fonts_names_iterator>
                                                  names_iterator;

  names_iterator names_begin, names_end;

  {
    auto names = xpp::xproto::request::list_fonts(*connection, n_fonts, 1, "*");

    names_begin = names_iterator::begin(names.get());
    names_end = names_iterator::end(names.get());
  }

  for (auto it = names_begin; it != names_end; ++it) {
    std::cerr << it->length() << ": " << *it << std::endl;
  }
  */

  return EXIT_SUCCESS;
}


  // auto it = names_begin;
  // std::cerr << "it  : " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // ++it;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // ++it;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it++;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it++;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it++;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it++;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // ++it;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it++;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // ++it;
  // std::cerr << "it++: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // it--;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // --it;
  // std::cerr << "it--: " << xcb_str_name_length(&*it) << ": "
  //           << std::string((char *)xcb_str_name(&*it), xcb_str_name_length(&*it))
  //           << std::endl;

  // {
  //   std::cerr << "fonts (native)" << std::endl;
  //   xcb_list_fonts_cookie_t cookie = xcb_list_fonts(*connection, n_fonts, 1, "*");
  //   xcb_list_fonts_reply_t * reply = xcb_list_fonts_reply(*connection, cookie, NULL);
  //   xcb_str_iterator_t iter = xcb_list_fonts_names_iterator(reply);
  //   xcb_str_t * name, * R, * prev;
  //   int len1, len2, len3, len4;

  //   // for ( ; iter.rem; xcb_str_next(&iter)) {
  //   //   xcb_str_t * name = iter.data;
  //   //   std::cerr << xcb_str_name_length(name) << ": " << std::string((char *)xcb_str_name(name), xcb_str_name_length(name)) << std::endl;
  //   // }

  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   len1 = xcb_str_sizeof(iter.data);
  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   len2 = xcb_str_sizeof(iter.data);
  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   len3 = xcb_str_sizeof(iter.data);
  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   len4 = xcb_str_sizeof(iter.data);
  //   xcb_str_next(&iter);
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;




  //   ++iter.rem;
  //   R = iter.data;
  //   // xcb_str_t * prev = R - xcb_str_sizeof(R);
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_sizeof(R));
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R)) - sizeof(xcb_str_t) - 1;
  //   prev = (xcb_str_t *)((char *)R) - len4;
  //   iter.index = (char *) iter.data - (char *) prev;
  //   iter.data = prev;
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   ++iter.rem;
  //   R = iter.data;
  //   // xcb_str_t * prev = R - xcb_str_sizeof(R);
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R));
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R)) - sizeof(xcb_str_t) - 2;
  //   prev = (xcb_str_t *)((char *)R) - len3;
  //   iter.index = (char *) iter.data - (char *) prev;
  //   iter.data = prev;
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   ++iter.rem;
  //   R = iter.data;
  //   // xcb_str_t * prev = R - xcb_str_sizeof(R);
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R));
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R)) - sizeof(xcb_str_t);
  //   prev = (xcb_str_t *)((char *)R) - len2;
  //   iter.index = (char *) iter.data - (char *) prev;
  //   iter.data = prev;
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   ++iter.rem;
  //   R = iter.data;
  //   // xcb_str_t * prev = R - xcb_str_sizeof(R);
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R));
  //   // prev = (xcb_str_t *)(((char *)R) - xcb_str_name_length(R)) - sizeof(xcb_str_t);
  //   prev = (xcb_str_t *)((char *)R) - len1;
  //   iter.index = (char *) iter.data - (char *) prev;
  //   iter.data = prev;
  //   name = iter.data;
  //   std::cerr << std::endl;
  //   std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   std::cerr << "index: 0x" << std::hex << iter.index <<  " (" << std::dec << iter.index << ")" << std::endl;
  //   std::cerr << "rem: " << iter.rem << std::dec << std::endl;
  //   std::cerr << xcb_str_name_length(name) << ": "
  //             << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //             << std::endl;

  //   // xcb_str_next(&iter);
  //   // name = iter.data;
  //   // std::cerr << "data: 0x" << std::hex << iter.data << " (" << std::dec << (unsigned long)iter.data << ")" << std::endl;
  //   // std::cerr << "index: 0x" << std::hex << iter.index << std::dec << std::endl;
  //   // std::cerr << xcb_str_name_length(name) << ": "
  //   //           << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //   //           << std::endl;

  //   // ++iter.rem;
  //   // R = iter.data;
  //   // prev = R - xcb_str_sizeof(R);
  //   // iter.index = (char *) iter.data - (char *) prev;
  //   // iter.data = prev;
  //   // name = iter.data;
  //   // std::cerr << "data: 0x" << std::hex << iter.data << "(" << std::dec << iter.data << ")" << std::endl;
  //   // std::cerr << "index: 0x" << std::hex << iter.index << std::endl;
  //   // std::cerr << xcb_str_name_length(name) << ": "
  //   //           << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //   //           << std::endl;

  //   // xcb_str_next(&iter);
  //   // name = iter.data;
  //   // std::cerr << "data: 0x" << std::hex << iter.data << "(" << std::dec << iter.data << ")" << std::endl;
  //   // std::cerr << "index: 0x" << std::hex << iter.index << std::endl;
  //   // std::cerr << xcb_str_name_length(name) << ": "
  //   //           << std::string((char *)xcb_str_name(name), xcb_str_name_length(name))
  //   //           << std::endl;

  //   std::cerr << "length: " << xcb_list_fonts_names_length(reply) << std::endl;
  // }

