#ifndef XPP_CORE_HPP
#define XPP_CORE_HPP

#include <string>
#include <memory>
#include <stdexcept>
#include <xcb/xcb.h>

namespace xpp {

class connection_error
  : public std::runtime_error
{
  public:
    connection_error(uint8_t code, const std::string & description)
      : std::runtime_error(description + "(" + std::to_string(code) + ")")
      , m_code(code)
      , m_description(description)
    {}

    uint8_t
    code(void)
    {
      return m_code;
    }

    std::string
    description(void)
    {
      return m_description;
    }

  protected:
    uint8_t m_code;
    std::string m_description;
};

class core
{
  protected:
    using shared_generic_event_ptr = std::shared_ptr<xcb_generic_event_t>;

    int m_screen = 0;
    // reference counting for xcb_connection_t
    std::shared_ptr<xcb_connection_t> m_c;

    shared_generic_event_ptr
    dispatch(const std::string & producer, xcb_generic_event_t * event) const
    {
      if (event) {
        if (event->response_type == 0) {
          throw std::shared_ptr<xcb_generic_error_t>(
              reinterpret_cast<xcb_generic_error_t *>(event));
        }

        return shared_generic_event_ptr(event, std::free);
      }

      check_connection();
      throw std::runtime_error(producer + " failed");
    }

  public:
    explicit
    core(xcb_connection_t * c)
      : m_c(std::shared_ptr<xcb_connection_t>(c, [](...) {}))
    {}

    template<typename ... ConnectionParameter>
    explicit
    core(xcb_connection_t * (*Connect)(ConnectionParameter ...),
               ConnectionParameter ... connection_parameter)
      : m_c(std::shared_ptr<xcb_connection_t>(
          Connect(connection_parameter ...),
          [&](void *) { disconnect(); }))
    {}

    // xcb_connect (const char *displayname, int *screenp)
    explicit
    core(const std::string & displayname = "")
      : core(xcb_connect, displayname.c_str(), &m_screen)
    {}

    // xcb_connect_to_fd (int fd, xcb_auth_info_t *auth_info)
    explicit
    core(int fd, xcb_auth_info_t * auth_info)
      : core(xcb_connect_to_fd, fd, auth_info)
    {}

    // xcb_connect_to_display_with_auth_info (
    //     const char *display, xcb_auth_info_t *auth, int *screen)
    explicit
    core(const std::string & display, xcb_auth_info_t * auth)
      : core(xcb_connect_to_display_with_auth_info,
                   display.c_str(), auth, &m_screen)
    {}

    virtual
    ~core(void)
    {}

    virtual
    xcb_connection_t *
    operator*(void) const
    {
      return m_c.get();
    }

    virtual
    operator xcb_connection_t *(void) const
    {
      return m_c.get();
    }

    virtual
    int
    default_screen(void) const
    {
      return m_screen;
    }

    virtual
    int
    flush(void) const
    {
      return xcb_flush(m_c.get());
    }

    virtual
    uint32_t
    get_maximum_request_length(void) const
    {
      return xcb_get_maximum_request_length(m_c.get());
    }

    virtual
    void
    prefetch_maximum_request_length(void) const
    {
      xcb_prefetch_maximum_request_length(m_c.get());
    }

    virtual
    shared_generic_event_ptr
    wait_for_event(void) const
    {
      return dispatch("wait_for_event", xcb_wait_for_event(m_c.get()));
    }

    virtual
    shared_generic_event_ptr
    poll_for_event(void) const
    {
      return shared_generic_event_ptr(xcb_poll_for_event(m_c.get()));
    }

    virtual
    shared_generic_event_ptr
    poll_for_queued_event(void) const
    {
      return shared_generic_event_ptr(xcb_poll_for_queued_event(m_c.get()));
    }

    virtual
    shared_generic_event_ptr
    poll_for_special_event(xcb_special_event_t * se) const
    {
      return shared_generic_event_ptr(xcb_poll_for_special_event(m_c.get(), se));
    }

    // virtual
    // shared_generic_event_ptr
    // poll_for_special_event(const std::shared_ptr<xcb_special_event_t> & se) const
    // {
    //   return poll_for_special_event(se.get());
    // }

    virtual
    shared_generic_event_ptr
    wait_for_special_event(xcb_special_event_t * se) const
    {
      return dispatch("wait_for_special_event",
                      xcb_wait_for_special_event(m_c.get(), se));
    }

    // virtual
    // shared_generic_event_ptr
    // wait_for_special_event(const std::shared_ptr<xcb_special_event_t> & se) const
    // {
    //   return wait_for_special_event(se.get());
    // }

    // xcb_special_event_t has incomplete type -> no std::shared_ptr
    virtual
    xcb_special_event_t *
    register_for_special_xge(xcb_extension_t * ext,
                             uint32_t eid,
                             uint32_t * stamp) const
    {
      return xcb_register_for_special_xge(m_c.get(), ext, eid, stamp);
    }

    virtual
    void
    unregister_for_special_event(xcb_special_event_t * se) const
    {
      xcb_unregister_for_special_event(m_c.get(), se);
    }

    virtual
    std::shared_ptr<xcb_generic_error_t>
    request_check(xcb_void_cookie_t cookie) const
    {
      return std::shared_ptr<xcb_generic_error_t>(
          xcb_request_check(m_c.get(), cookie));
    }

    virtual
    void
    discard_reply(unsigned int sequence) const
    {
      xcb_discard_reply(m_c.get(), sequence);
    }

    // The result must not be freed.
    // This storage is managed by the cache itself.
    virtual
    const xcb_query_extension_reply_t *
    get_extension_data(xcb_extension_t * ext) const
    {
      return xcb_get_extension_data(m_c.get(), ext);
    }

    virtual
    void
    prefetch_extension_data(xcb_extension_t * ext) const
    {
      xcb_prefetch_extension_data(m_c.get(), ext);
    }

    virtual
    const xcb_setup_t *
    get_setup(void) const
    {
      return xcb_get_setup(m_c.get());
    }

    virtual
    int
    get_file_descriptor(void) const
    {
      return xcb_get_file_descriptor(m_c.get());
    }

    virtual
    int
    connection_has_error(void) const
    {
      return xcb_connection_has_error(m_c.get());
    }

    virtual
    void
    disconnect(void) const
    {
      xcb_disconnect(m_c.get());
    }

    // hostname, display, screen
    virtual
    std::tuple<std::string, int, int>
    parse_display(const std::string & name) const
    {
      int screen = 0;
      int display = 0;
      char * host = NULL;
      std::string hostname;

      xcb_parse_display(name.c_str(), &host, &display, &screen);
      if (host != NULL) {
        hostname = std::string(host);
      }

      return std::make_tuple(hostname, display, screen);
    }

    virtual
    uint32_t
    generate_id(void) const
    {
      return xcb_generate_id(m_c.get());
    }

    xcb_screen_t *
    screen_of_display(int screen)
    {
      xcb_screen_iterator_t iter;

      iter = xcb_setup_roots_iterator(xcb_get_setup(m_c.get()));
      for (; iter.rem; --screen, xcb_screen_next(&iter))
        if (screen == 0)
          return iter.data;

      return NULL;
    }

    void
    check_connection(void) const
    {
      switch (xcb_connection_has_error(m_c.get())) {
        case XCB_CONN_ERROR:
          throw(connection_error(
                XCB_CONN_ERROR, "XCB_CONN_ERROR"));

        case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
          throw(connection_error(XCB_CONN_CLOSED_EXT_NOTSUPPORTED,
                                 "XCB_CONN_CLOSED_EXT_NOTSUPPORTED"));

        case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
          throw(connection_error(XCB_CONN_CLOSED_MEM_INSUFFICIENT,
                                 "XCB_CONN_CLOSED_MEM_INSUFFICIENT"));

        case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
          throw(connection_error(XCB_CONN_CLOSED_REQ_LEN_EXCEED,
                                 "XCB_CONN_CLOSED_REQ_LEN_EXCEED"));

        case XCB_CONN_CLOSED_PARSE_ERR:
          throw(connection_error(XCB_CONN_CLOSED_PARSE_ERR,
                                 "XCB_CONN_CLOSED_PARSE_ERR"));

        case XCB_CONN_CLOSED_INVALID_SCREEN:
          throw(connection_error(XCB_CONN_CLOSED_INVALID_SCREEN,
                                 "XCB_CONN_CLOSED_INVALID_SCREEN"));
      };
    }
}; // class core

} // namespace xpp

#endif // XPP_CORE_HPP
