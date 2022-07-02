#ifndef XPP_PIXMAP_HPP
#define XPP_PIXMAP_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class pixmap
  : public xpp::generic::resource<Connection, xcb_pixmap_t,
                                  xpp::x::pixmap, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_pixmap_t,
                                        xpp::x::pixmap, Interfaces ...>;

    template<typename C, typename Create, typename Destroy>
    pixmap(C && c, Create && create, Destroy && destroy)
      : base(base::make(std::forward<C>(c),
                        std::forward<Create>(create),
                        std::forward<Destroy>(destroy)))
    {}

  public:
    using base::base;
    using base::operator=;

    template<typename C>
    static
    pixmap<Connection, Interfaces ...>
    create(C && c, uint8_t depth, xcb_drawable_t drawable,
                   uint16_t width, uint16_t height)
    {
      return pixmap(
        std::forward<C>(c),
        [&](const Connection & c, const xcb_pixmap_t & pixmap)
        {
          xpp::x::create_pixmap(c, depth, pixmap, drawable, width, height);
        },
        [&](const Connection & c, const xcb_pixmap_t & pixmap)
        {
          xpp::x::free_pixmap(c, pixmap);
        });
    }

    template<typename C>
    static
    pixmap<Connection, Interfaces ...>
    create_checked(C && c, uint8_t depth, xcb_drawable_t drawable,
                   uint16_t width, uint16_t height)
    {
      return pixmap(
        std::forward<C>(c),
        [&](const Connection & c, const xcb_pixmap_t & pixmap)
        {
          xpp::x::create_pixmap_checked(c, depth, pixmap, drawable, width, height);
        },
        [&](const Connection & c, const xcb_pixmap_t & pixmap)
        {
          xpp::x::free_pixmap_checked(c, pixmap);
        });
    }
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::pixmap<Connection, Interfaces ...>>
{
  typedef xcb_pixmap_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_PIXMAP_HPP
