#ifndef XPP_GENERIC_EXTENSION_HPP
#define XPP_GENERIC_EXTENSION_HPP

// #include <iostream>
#include <xcb/xcb.h>

namespace xpp { namespace generic {

template<typename Derived, xcb_extension_t * Id>
class extension
{
  public:
    extension(xcb_connection_t * const c)
      : m_c(c)
    {
      prefetch();
    }

    const xcb_query_extension_reply_t &
    operator*(void) const
    {
      return *m_extension;
    }

    const xcb_query_extension_reply_t *
    operator->(void) const
    {
      return m_extension;
    }

    operator const xcb_query_extension_reply_t *(void) const
    {
      return m_extension;
    }

    Derived &
    get(void)
    {
      m_extension = xcb_get_extension_data(m_c, Id);
      return static_cast<Derived &>(*this);
    }

    Derived &
    prefetch(void)
    {
      xcb_prefetch_extension_data(m_c, Id);
      return static_cast<Derived &>(*this);
    }

  private:
    xcb_connection_t * m_c = nullptr;
    // The result must not be freed.
    // This storage is managed by the cache itself.
    const xcb_query_extension_reply_t * m_extension = nullptr;
}; // class extension

} } // namespace xpp::generic

#endif // XPP_GENERIC_EXTENSION_HPP
