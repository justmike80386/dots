#ifndef XPP_DRAWABLE_HPP
#define XPP_DRAWABLE_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class drawable
  : public xpp::generic::resource<Connection, xcb_drawable_t,
                                  xpp::x::drawable, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_drawable_t,
                                        xpp::x::drawable, Interfaces ...>;

  public:
    using base::base;
    using base::operator=;
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::drawable<Connection, Interfaces ...>>
{
  typedef xcb_drawable_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_DRAWABLE_HPP
