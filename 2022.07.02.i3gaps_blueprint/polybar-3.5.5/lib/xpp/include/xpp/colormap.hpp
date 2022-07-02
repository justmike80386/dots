#ifndef XPP_COLORMAP_HPP
#define XPP_COLORMAP_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class colormap
  : public xpp::generic::resource<Connection, xcb_colormap_t,
                                  xpp::x::colormap, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_colormap_t,
                                        xpp::x::colormap, Interfaces ...>;

    template<typename C, typename Create, typename Destroy>
    colormap(C && c, Create && create, Destroy && destroy)
      : base(base::make(std::forward<C>(c),
                        std::forward<Create>(create),
                        std::forward<Destroy>(destroy)))
    {}

  public:
    using base::base;
    using base::operator=;

    template<typename C>
    static
    colormap<Connection, Interfaces ...>
    create(C && c, uint8_t alloc, xcb_window_t window, xcb_visualid_t visual)
    {
      return colormap(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_colormap_t & colormap)
          {
            xpp::x::create_colormap(c, alloc, colormap, window, visual);
          },
          [&](const Connection & c, const xcb_colormap_t & colormap)
          {
            xpp::x::free_colormap(c, colormap);
          });
    }

    template<typename C>
    static
    colormap<Connection, Interfaces ...>
    create_checked(C && c, uint8_t alloc,
                   xcb_window_t window, xcb_visualid_t visual)
    {
      return colormap(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_colormap_t & colormap)
          {
            xpp::x::create_colormap_checked(c, alloc, colormap, window, visual);
          },
          [&](const Connection & c, const xcb_colormap_t & colormap)
          {
            xpp::x::free_colormap_checked(c, colormap);
          });
    }
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::colormap<Connection, Interfaces ...>>
{
  typedef xcb_colormap_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_COLORMAP_HPP
