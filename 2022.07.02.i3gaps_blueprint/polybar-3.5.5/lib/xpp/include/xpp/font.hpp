#ifndef XPP_FONT_HPP
#define XPP_FONT_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class font
  : public xpp::generic::resource<Connection, xcb_font_t,
                                  xpp::x::font, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_font_t,
                                        xpp::x::font, Interfaces ...>;

    template<typename C, typename Create, typename Destroy>
    font(C && c, Create && create, Destroy && destroy)
      : base(base::make(std::forward<C>(c),
                        std::forward<Create>(create),
                        std::forward<Destroy>(destroy)))
    {}

  public:
    using base::base;
    using base::operator=;

    template<typename C>
    static
    font<Connection, Interfaces ...>
    open(C && c, const std::string & name) noexcept
    {
      return font(std::forward<C>(c),
                  [&](const Connection & c, const xcb_font_t & font)
                  {
                    xpp::x::open_font(c, font, name);
                  },
                  [&](const Connection & c, const xcb_font_t & font)
                  {
                    xpp::x::close_font(c, font);
                  });
    }

    template<typename C>
    static
    font<Connection, Interfaces ...>
    open_checked(C && c, const std::string & name)
    {
      return font(std::forward<C>(c),
                  [&](const Connection & c, const xcb_font_t & font)
                  {
                    xpp::x::open_font_checked(c, font, name);
                  },
                  [&](const Connection & c, const xcb_font_t & font)
                  {
                    xpp::x::close_font_checked(c, font);
                  });
    }
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::font<Connection, Interfaces ...>>
{
  typedef xcb_font_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_FONT_HPP
