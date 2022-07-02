#include <iostream>
#include <string>

#include <xcb/xcb.h>

#include "../event.hpp"
#include "../core/value_iterator.hpp"
#include "../core/connection.hpp"
#include "../core/extension.hpp"
#include "../core/error.hpp"

#include "../request.hpp"
#include "../request_ng.hpp"

#define CHECKED_REQUEST(NAME) \
  decltype(NAME ## _reply), NAME ## _reply, \
  decltype(NAME), NAME

#define UNCHECKED_REQUEST(NAME) \
  decltype(NAME ## _reply), NAME ## _reply, \
  decltype(NAME ## _unchecked), NAME ## _unchecked

namespace xpp { namespace generic {

// class get_screen_info_checked
//   : public checked_reply_request<xpp::connection<xpp::extension::randr>,
//                                  xpp::extension::randr,
//                                  CHECKED_REQUEST(xcb_randr_get_screen_info)>
// {
//   public:
//     typedef checked_reply_request<xpp::connection<xpp::extension::randr>,
//                                   xpp::extension::randr,
//                                   CHECKED_REQUEST(xcb_randr_get_screen_info)>
//                                     base;
//     using base::base;
// };

// class get_screen_info_unchecked
//   : public unchecked_reply_request<UNCHECKED_REQUEST(xcb_randr_get_screen_info)>
// {
//   public:
//     typedef unchecked_reply_request<UNCHECKED_REQUEST(xcb_randr_get_screen_info)>
//                                     base;
//     using base::base;
// };

// template<typename NoThrow = void>
// class get_screen_info
//   : public std::conditional<std::is_same<NoThrow, std::nothrow_t>::value,
//                             get_screen_info_unchecked,
//                             get_screen_info_checked>::type
// {
//   public:
//     typedef typename std::conditional<
//       std::is_same<NoThrow, std::nothrow_t>::value,
//       get_screen_info_unchecked,
//       get_screen_info_checked>::type
//         base;
//     using base::base;
// };

/*
class query_tree_checked
  : public checked_reply_request<xpp::connection<>, void,
                                 CHECKED_REQUEST(xcb_query_tree)>
{
  public:
    typedef checked_reply_request<xpp::connection<>, void,
                                  CHECKED_REQUEST(xcb_query_tree)>
                                    base;
    using base::base;
};

class query_tree_unchecked
  : public unchecked_reply_request<UNCHECKED_REQUEST(xcb_query_tree)>
{
  public:
    typedef unchecked_reply_request<UNCHECKED_REQUEST(xcb_query_tree)>
                                    base;
    using base::base;
};

template<typename NoThrow = void>
class query_tree
  : public std::conditional<std::is_same<NoThrow, std::nothrow_t>::value,
                            query_tree_unchecked,
                            query_tree_checked
    >::type
{
  public:
    typedef typename std::conditional<
      std::is_same<NoThrow, std::nothrow_t>::value,
      query_tree_unchecked,
      query_tree_checked
        >::type
        base;
    using base::base;
};
*/

// class query_tree_checked
//   : public xpp::generic::checked::request<xpp::extension::x,
//                                              CHECKED_REQUEST(xcb_query_tree)>
// {
//   typedef xpp::generic::checked::request<xpp::extension::x,
//                                             CHECKED_REQUEST(xcb_query_tree)>
//                                               base;
//   using base::base;
// };

// class map_window_checked
//   : public ::test::generic::checked::request<xpp::extension::x,
//                                              CHECKED_REQUEST(xcb_map_window)>
// {
//   typedef ::test::generic::checked::request<xpp::extension::x,
//                                             CHECKED_REQUEST(xcb_map_window)>
//                                               base;
//   using base::base;
// };

// class get_screen_info
//   // : public wrapper<xpp::connection<xpp::extension::randr>, xpp::extension::randr,
//   : public wrapper<xpp::connection<xpp::extension::randr>,
//                    xpp::extension::randr, xpp::extension::x,
//                    decltype(xcb_randr_get_screen_info_reply), xcb_randr_get_screen_info_reply,
//                    decltype(xcb_randr_get_screen_info), xcb_randr_get_screen_info>
// {
//   public:
//     // typedef wrapper<xpp::extension::randr,
//     typedef wrapper<xpp::connection<xpp::extension::randr>,
//                     xpp::extension::randr, xpp::extension::x,
//                     decltype(xcb_randr_get_screen_info_reply), xcb_randr_get_screen_info_reply,
//                     decltype(xcb_randr_get_screen_info), xcb_randr_get_screen_info>
//                       base;
//     using base::base;
// };

// class query_tree
//   // : public wrapper<xpp::extension::x,
//   : public wrapper<xpp::connection<>,
//                    xpp::extension::x, xpp::extension::x,
//                    decltype(xcb_query_tree_reply), xcb_query_tree_reply,
//                    decltype(xcb_query_tree), xcb_query_tree>
// {
//   public:
//     typedef wrapper<xpp::connection<>,
//                     xpp::extension::x, xpp::extension::x,
//                     decltype(xcb_query_tree_reply), xcb_query_tree_reply,
//                     decltype(xcb_query_tree), xcb_query_tree>
//                       base;
//     using base::base;
// };

// class dpms_capable
//   // : public wrapper<// xpp::connection<xpp::extension::dpms>, xpp::extension::dpms,
//   : public wrapper<xpp::extension::dpms,
//                    decltype(xcb_dpms_capable_reply), xcb_dpms_capable_reply,
//                    decltype(xcb_dpms_capable), xcb_dpms_capable>
// {
//   public:
//     // typedef wrapper<// xpp::connection<xpp::extension::dpms>, xpp::extension::dpms,
//     typedef wrapper<xpp::extension::dpms,
//                     decltype(xcb_dpms_capable_reply), xcb_dpms_capable_reply,
//                     decltype(xcb_dpms_capable), xcb_dpms_capable>
//                       base;
//     using base::base;
// };

// class render_query_pict_index_values
//   // : public wrapper<// xpp::connection<xpp::extension::render>, xpp::extension::render,
//   : public wrapper<xpp::extension::render,
//                    decltype(xcb_render_query_pict_index_values_reply), xcb_render_query_pict_index_values_reply,
//                    decltype(xcb_render_query_pict_index_values), xcb_render_query_pict_index_values>
// {
//   public:
//     // typedef wrapper<// xpp::connection<xpp::extension::render>, xpp::extension::render,
//     typedef wrapper<xpp::extension::render,
//                     decltype(xcb_render_query_pict_index_values_reply), xcb_render_query_pict_index_values_reply,
//                     decltype(xcb_render_query_pict_index_values), xcb_render_query_pict_index_values>
//                       base;
//     using base::base;
// };

// class render_query_filters
//   // : public wrapper<xpp::extension::render,
//   : public wrapper<xpp::connection<xpp::extension::render>,
//                    xpp::extension::render, xpp::extension::x,
//                    decltype(xcb_render_query_filters_reply), xcb_render_query_filters_reply,
//                    decltype(xcb_render_query_filters), xcb_render_query_filters>
// {
//   public:
//     // typedef wrapper<xpp::extension::render,
//     typedef wrapper<xpp::connection<xpp::extension::render>,
//                     xpp::extension::render, xpp::extension::x,
//                     decltype(xcb_render_query_filters_reply), xcb_render_query_filters_reply,
//                     decltype(xcb_render_query_filters), xcb_render_query_filters>
//                       base;
//     using base::base;
// };

// template<>
// class query_tree<xcb_connection_t>
//   : public wrapper<xcb_connection_t, void,
//                    decltype(xcb_query_tree_reply), xcb_query_tree_reply,
//                    decltype(xcb_query_tree), xcb_query_tree>
// {
//   public:
//     typedef wrapper<xcb_connection_t, void,
//                     decltype(xcb_query_tree_reply), xcb_query_tree_reply,
//                     decltype(xcb_query_tree), xcb_query_tree>
//                       base;
//     using base::base;
// };

}; }; // namespace xpp::generic

void error_dispatcher(xcb_generic_error_t * error)
{
  switch (error->error_code) {
    case XCB_WINDOW:
      xcb_window_error_t * e = (xcb_window_error_t *)error;
      std::cerr << "XCB_WINDOW with bad_value: " << e->bad_value << std::endl;
      // throw xpp::error::generic<XCB_WINDOW, xcb_window_error_t>(error);
      throw xpp::generic::error<XCB_WINDOW, xcb_window_error_t>(error);
      // throw xpp::x::window_error(error);
      break;
  }
}

void check_error_direct(xcb_generic_error_t * error)
{
  if (error) {
    std::cerr << "error code: " << (int)error->error_code << std::endl;
    try {
    error_dispatcher(error);
    } catch (const std::exception & exception) {
      std::cerr << "caught exception: " << exception.what() << std::endl;
    }
  } else {
    std::cerr << "no error occurred" << std::endl;
  }
}

void check_error_event(xcb_connection_t * c)
{
  xcb_generic_event_t * event = xcb_poll_for_event(c);
  xcb_flush(c);
  if (event) {
    uint8_t response = event->response_type & ~0x80;
    std::cerr << "event response: " << (int)response << std::endl;
    check_error_direct((xcb_generic_error_t *)event);
  } else {
    std::cerr << "no event available" << std::endl;
  }
}

void
map_window(xcb_connection_t * c, xcb_window_t window)
{
  xcb_map_window(c, window);
}

template<typename Extension>
void
map_window(const Extension & e, xcb_connection_t * c, xcb_window_t window)
{
  typedef typename Extension::error_dispatcher dispatcher;
  dispatcher(/* e->first_error */)(
      xcb_request_check(c, xcb_map_window_checked(c, window)));
}

template<typename ... Parameters>
void
map_window(const xpp::connection<Parameters ...> & c, xcb_window_t window)
{
  map_window(static_cast<const xpp::extension::x &>(c), c, window);
}

int main(int argc, char ** argv)
{
  xpp::connection<xpp::extension::randr,
                  xpp::extension::render,
                  xpp::extension::dpms> c("");
  // xpp::connection<> c("");

  // auto tree_1 = xpp::generic::query_tree<decltype(c), xpp::extension::x>(c, 0);
  // auto tree_1 = xpp::generic::query_tree<decltype(c)>(c, 0);
  // auto tree_1 = xpp::generic::query_tree<>(static_cast<xcb_connection_t *>(c), 0);

  auto & randr  = static_cast<xpp::randr::protocol &>(c);
  auto & render = static_cast<xpp::render::protocol &>(c);
  auto & dpms   = static_cast<xpp::dpms::protocol &>(c);

  auto & randr_ext  = static_cast<xpp::extension::randr &>(c);
  auto & render_ext = static_cast<xpp::extension::render &>(c);
  auto & dpms_ext   = static_cast<xpp::extension::dpms &>(c);

  *randr.query_version(XCB_RANDR_MAJOR_VERSION, XCB_RANDR_MINOR_VERSION);
  *render.query_version(XCB_RENDER_MAJOR_VERSION, XCB_RENDER_MINOR_VERSION);
  *dpms.get_version(XCB_DPMS_MAJOR_VERSION, XCB_DPMS_MINOR_VERSION);

  std::cerr << "randr first_error: " << (int)randr_ext->first_error << std::endl;
  std::cerr << "render first_error: " << (int)render_ext->first_error << std::endl;
  std::cerr << "dpms first_error: " << (int)dpms_ext->first_error << std::endl;

  // auto tree = xpp::generic::query_tree(c, 0);
  // std::cerr << "tree go" << std::endl;
  // try {
  //   tree.get();
  //   auto error = tree.error();
  //   std::cerr << error.get() << std::endl;
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "tree fin" << std::endl << std::endl;

  // auto screen_info = xpp::generic::get_screen_info(c, 0);
  // std::cerr << "screen_info go" << std::endl;
  // try {
  //   screen_info.get();
  //   auto error = screen_info.error();
  //   std::cerr << error.get() << std::endl;
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "screen_info fin" << std::endl << std::endl;

  // auto dpms_capable = xpp::generic::dpms_capable(c);
  // std::cerr << "dpms_capable go" << std::endl;
  // try {
  //   dpms_capable.get();
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "dpms_capable fin" << std::endl;

  // auto pict_index_values = xpp::generic::render_query_pict_index_values(c, -1);
  // std::cerr << "pict_index_values go" << std::endl;
  // try {
  //   pict_index_values.get();
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "pict_index_values fin" << std::endl;

  // auto filters = xpp::generic::render_query_filters(c, 0);
  // std::cerr << "filters go" << std::endl;
  // try {
  //   filters.get();
  //   auto error = filters.error();
  //   std::cerr << error.get() << std::endl;
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "filters fin" << std::endl << std::endl;



  // // auto tree_2 = xpp::generic::query_tree<xcb_connection_t>(c, 0);
  // auto tree_2 = xpp::generic::query_tree<>(c, 0);
  // std::cerr << "tree_2 go" << std::endl;
  // try {
  //   tree_2.get();
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }
  // std::cerr << "tree_2 fin" << std::endl;

  // auto reply = xpp::request::x::query_tree(c, 0);
  // reply.get();
  // reply.check((xpp::extension::x &)c);

  // auto reply = xpp::request::x::query_tree(c, 0);
  // reply.get();
  // try {
  //   reply.check((xpp::extension::x &)c);
  // } catch (const std::exception & exception) {
  //   std::cerr << "Exception: " << exception.what() << std::endl;
  // }


  // return 0;

  /*
  map_window(*c, 0);

  try {
    map_window(c, 0);
  } catch (...) {}

  try {
    map_window((xpp::extension::x &)c, c, 0);
    map_window((xpp::extension::x &)c, c, 0);
    // map_window((xpp::extension::randr &)c, c, 0);
  } catch (...) {}
  */

  // xcb_generic_error_t * generic_error = NULL;
  // xcb_query_tree_reply_t * query_tree_reply = NULL;

  // checked request with reply, error directly available
  {
  std::cerr << "checked request with reply, error directly available" << std::endl;
  // xcb_query_tree_cookie_t query_tree_cookie = xcb_query_tree(c, 5);
  // query_tree_reply = xcb_query_tree_reply(c, query_tree_cookie, &generic_error);
  try {
    auto screen_info = xpp::request::randr::get_screen_info(c, 0);
    // auto tree = xpp::generic::query_tree<>(c, 5);
    screen_info.get();
  } catch (const xpp::x::error::window & e) {
    std::cerr << "xpp::x::error::window: " << e.what() << std::endl;
  } catch (const std::exception & e) {
    std::cerr << "std::exception: " << e.what() << std::endl;
  }
  // auto reply = tree.get();
  c.flush();
  // std::cerr << "query_tree_reply: " << reply.get() << std::endl;
  // check_error_direct(tree.error());
  // std::cerr << "query_tree_reply: " << query_tree_reply << std::endl;
  // query_tree_reply = NULL;
  // check_error_direct(generic_error);
  // check_error_direct(generic_error);
  // generic_error = NULL;
  }
  std::cerr << std::endl;

  // checked request with reply, error directly available
  {
  std::cerr << "checked request with reply, error directly available" << std::endl;
  // xcb_query_tree_cookie_t query_tree_cookie = xcb_query_tree(c, 5);
  // query_tree_reply = xcb_query_tree_reply(c, query_tree_cookie, &generic_error);
  try {
    auto tree = xpp::request::x::query_tree(c, 5);
    // auto tree = xpp::generic::query_tree<>(c, 5);
    tree.get();
  } catch (const xpp::x::error::window & e) {
    std::cerr << "xpp::x::error::window: " << e.what() << std::endl;
  } catch (const std::exception & e) {
    std::cerr << "std::exception: " << e.what() << std::endl;
  }
  // auto reply = tree.get();
  c.flush();
  // std::cerr << "query_tree_reply: " << reply.get() << std::endl;
  // check_error_direct(tree.error());
  // std::cerr << "query_tree_reply: " << query_tree_reply << std::endl;
  // query_tree_reply = NULL;
  // check_error_direct(generic_error);
  // check_error_direct(generic_error);
  // generic_error = NULL;
  }
  std::cerr << std::endl;

  // // unchecked request with reply, error in event queue
  // {
  // std::cerr << "unchecked request with reply, error in event queue" << std::endl;
  // // xcb_query_tree_cookie_t query_tree_cookie = xcb_query_tree_unchecked(c, 17);
  // // query_tree_reply = xcb_query_tree_reply(c, query_tree_cookie, &generic_error);
  // auto tree = xpp::generic::query_tree<std::nothrow_t>(c, 5);
  // auto reply = tree.get();
  // c.flush();
  // std::cerr << "query_tree_reply: " << reply.get() << std::endl;
  // // std::cerr << "query_tree_reply: " << query_tree_reply << std::endl;
  // // std::cerr << "query_tree_reply error: " << generic_error << std::endl;
  // // query_tree_reply = NULL;
  // check_error_event(c);
  // }
  // std::cerr << std::endl;

  // // checked request without reply, error directly available
  // {
  // std::cerr << "checked request without reply, error directly available" << std::endl;
  // // xcb_void_cookie_t void_cookie = xcb_map_window_checked(c, 42);
  // try {
  //   ::test::map_window(c, 0);
  //   // xpp::request::x::map_window(c, 0);
  // } catch (const xpp::x::error::window & e) {
  //   std::cerr << "xpp::x::error::window: " << e.what() << std::endl;
  // } catch (const std::exception & e) {
  //   std::cerr << "std::exception: " << e.what() << std::endl;
  // }
  // c.flush();
  // // generic_error = xcb_request_check(c, void_cookie);
  // // check_error_direct(generic_error);
  // // generic_error = NULL;
  // }
  // std::cerr << std::endl;

  // unchecked request without reply, error in event queue
  {
  std::cerr << "unchecked request without reply, error in event queue" << std::endl;
  xcb_map_window(c, 0);
  c.flush();
  check_error_event(c);
  }

  return 0;
}
