#ifndef XPP_CONNECTION_HPP
#define XPP_CONNECTION_HPP

#include "core.hpp"
#include "generic/factory.hpp"

#include "xpp/proto/x.hpp"

namespace xpp {

namespace detail {

template<typename Connection, typename ... Extensions>
class interfaces
  : public xpp::x::extension::interface<interfaces<Connection, Extensions ...>, Connection>
  , public Extensions::template interface<interfaces<Connection, Extensions ...>, Connection> ...
{
  public:
    Connection
    connection(void) const
    {
      return static_cast<const Connection &>(*this);
    }
}; // class interfaces

} // namespace detail

template<typename ... Extensions>
class connection
  : public xpp::core
  , public xpp::generic::error_dispatcher
  , public detail::interfaces<connection<Extensions ...>, Extensions ...>
  // private interfaces: extensions and error_dispatcher
  , private xpp::x::extension
  , private xpp::x::extension::error_dispatcher
  , private Extensions ...
  , private Extensions::error_dispatcher ...
{
  protected:
    typedef connection<Extensions ...> self;


  public:
    template<typename ... Parameters>
    explicit
    connection(Parameters && ... parameters)
      : xpp::core::core(std::forward<Parameters>(parameters) ...)
      , detail::interfaces<connection<Extensions ...>, Extensions ...>(*this)
      , Extensions(static_cast<xcb_connection_t *>(*this)) ...
      , Extensions::error_dispatcher(static_cast<Extensions &>(*this).get()) ...
    {
      m_root_window = screen_of_display(default_screen())->root;
    }

    virtual
    ~connection(void)
    {}

    virtual
    operator xcb_connection_t *(void) const
    {
      return *(static_cast<const core &>(*this));
    }

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      check<xpp::x::extension, Extensions ...>(error);
    }

    template<typename Extension>
    const Extension &
    extension(void) const
    {
      return static_cast<const Extension &>(*this);
    }

    // TODO
    // virtual operator Display * const(void) const
    // {
    // }

    template<typename Window = xcb_window_t>
    Window
    root(void)
    {
      using make = xpp::generic::factory::make<self, xcb_window_t, Window>;
      return make()(*this, m_root_window);
    }

    template<typename Window = xcb_window_t>
    Window
    root(void) const
    {
      using make = xpp::generic::factory::make<self, xcb_window_t, Window>;
      return make()(*this, m_root_window);
    }

    virtual
    shared_generic_event_ptr
    wait_for_event(void) const
    {
      try {
        return core::wait_for_event();
      } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
        check<xpp::x::extension, Extensions ...>(error);
      }
      // re-throw any exception caused by wait_for_event
      throw;
    }

    virtual
    shared_generic_event_ptr
    wait_for_special_event(xcb_special_event_t * se) const
    {
      try {
        return core::wait_for_special_event(se);
      } catch (const std::shared_ptr<xcb_generic_error_t> & error) {
        check<xpp::x::extension, Extensions ...>(error);
      }
      // re-throw any exception caused by wait_for_special_event
      throw;
    }

  private:
    xcb_window_t m_root_window;

    template<typename Extension, typename Next, typename ... Rest>
    void
    check(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      check<Extension>(error);
      check<Next, Rest ...>(error);
    }

    template<typename Extension>
    void
    check(const std::shared_ptr<xcb_generic_error_t> & error) const
    {
      using error_dispatcher = typename Extension::error_dispatcher;
      auto & dispatcher = static_cast<const error_dispatcher &>(*this);
      dispatcher(error);
    }
}; // class connection

template<>
template<typename ... Parameters>
connection<>::connection(Parameters && ... parameters)
  : xpp::core::core(std::forward<Parameters>(parameters) ...)
  , detail::interfaces<connection<>>(*this)
{
  m_root_window = screen_of_display(static_cast<core &>(*this).default_screen())->root;
}

} // namespace xpp

#endif // XPP_CONNECTION_HPP
