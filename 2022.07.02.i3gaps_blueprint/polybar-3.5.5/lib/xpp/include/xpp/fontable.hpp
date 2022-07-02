#ifndef XPP_FONTABLE_HPP
#define XPP_FONTABLE_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class fontable
  : public xpp::generic::resource<Connection, xcb_fontable_t,
                                  xpp::x::fontable, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_fontable_t,
                                        xpp::x::fontable, Interfaces ...>;

  public:
    using base::base;
    using base::operator=;
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::fontable<Connection, Interfaces ...>>
{
  typedef xcb_fontable_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_FONTABLE_HPP
