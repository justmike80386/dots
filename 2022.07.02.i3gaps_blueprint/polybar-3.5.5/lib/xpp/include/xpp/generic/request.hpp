#ifndef XPP_GENERIC_REQUEST_HPP
#define XPP_GENERIC_REQUEST_HPP

#include <array>
#include <memory>
#include <cstdlib>
#include <xcb/xcb.h>
#include "error.hpp"
#include "signature.hpp"

#define REPLY_TEMPLATE \
  typename Reply, \
  typename Cookie, \
  Reply *(&ReplyFunction)(xcb_connection_t *, Cookie, xcb_generic_error_t **)

#define REPLY_SIGNATURE \
  xpp::generic::signature<Reply *(xcb_connection_t *, \
                                  Cookie, \
                                  xcb_generic_error_t **), \
                          ReplyFunction>

#define REPLY_COOKIE_TEMPLATE \
  typename ... CookieParameter, \
  Cookie(&CookieFunction)(CookieParameter ...)

#define REPLY_COOKIE_SIGNATURE \
  xpp::generic::signature<Cookie(CookieParameter ...), CookieFunction>

namespace xpp { namespace generic {

template<typename Connection, typename Dispatcher>
void
check(Connection && c, const xcb_void_cookie_t & cookie)
{
  xcb_generic_error_t * error =
    xcb_request_check(std::forward<Connection>(c), cookie);
  if (error) {
    dispatch(std::forward<Connection>(c),
             std::shared_ptr<xcb_generic_error_t>(error, std::free));
  }
}

struct checked_tag {};
struct unchecked_tag {};

template<typename ... Types>
class reply;

template<typename Derived,
         typename Connection,
         typename Check,
         REPLY_TEMPLATE,
         REPLY_COOKIE_TEMPLATE>
class reply<Derived,
            Connection,
            Check,
            REPLY_SIGNATURE,
            REPLY_COOKIE_SIGNATURE>
{
  public:
    template<typename C, typename ... Parameter>
    reply(C && c, Parameter && ... parameter)
      : m_c(std::forward<C>(c))
      , m_cookie(Derived::cookie(std::forward<C>(c),
                                 std::forward<Parameter>(parameter) ...))
    {}

    operator bool(void)
    {
      return m_reply.operator bool();
    }

    const Reply &
    operator*(void)
    {
      return *get();
    }

    Reply *
    operator->(void)
    {
      return get().get();
    }

    const std::shared_ptr<Reply> &
    get(void)
    {
      if (! m_reply) {
        m_reply = get(Check());
      }
      return m_reply;
    }

    template<typename ... Parameter>
    static
    Cookie
    cookie(Parameter && ... parameter)
    {
      return CookieFunction(std::forward<Parameter>(parameter) ...);
    }

  protected:
    Connection m_c;
    Cookie m_cookie;
    std::shared_ptr<Reply> m_reply;

    std::shared_ptr<Reply>
    get(checked_tag)
    {
      xcb_generic_error_t * error = nullptr;
      auto reply = std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, &error),
                                          std::free);
      if (error) {
        dispatch(m_c, std::shared_ptr<xcb_generic_error_t>(error, std::free));
      }
      return reply;
    }

    std::shared_ptr<Reply>
    get(unchecked_tag)
    {
      return std::shared_ptr<Reply>(ReplyFunction(m_c, m_cookie, nullptr),
                                    std::free);
    }
};

} } // namespace xpp::generic

#endif // XPP_GENERIC_REQUEST_HPP
