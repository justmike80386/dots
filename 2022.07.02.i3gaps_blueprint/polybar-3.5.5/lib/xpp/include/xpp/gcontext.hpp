#ifndef XPP_GCONTEXT_HPP
#define XPP_GCONTEXT_HPP

#include "xpp/proto/x.hpp"
#include "generic/resource.hpp"

namespace xpp {

template<typename Connection, template<typename, typename> class ... Interfaces>
class gcontext
  : public xpp::generic::resource<Connection, xcb_gcontext_t,
                                  xpp::x::gcontext, Interfaces ...>
{
  protected:
    using base = xpp::generic::resource<Connection, xcb_gcontext_t,
                                        xpp::x::gcontext, Interfaces ...>;

    template<typename C, typename Create, typename Destroy>
    gcontext(C && c, Create && create, Destroy && destroy)
      : base(base::make(std::forward<C>(c),
                        std::forward<Create>(create),
                        std::forward<Destroy>(destroy)))
    {}

  public:
    using base::base;
    using base::operator=;

    template<typename C>
    static
    gcontext<Connection, Interfaces ...>
    create(C && c, xcb_drawable_t drawable,
           uint32_t value_mask, const uint32_t * value_list)
    {
      return gcontext(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::create_gc(c, gcontext, drawable, value_mask, value_list);
          },
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::free_gc(c, gcontext);
          });
    }

    template<typename C>
    static
    gcontext<Connection, Interfaces ...>
    create_checked(C && c, xcb_drawable_t drawable,
                   uint32_t value_mask, const uint32_t * value_list)
    {
      return gcontext(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::create_gc_checked(c, gcontext, drawable,
                                      value_mask, value_list);
          },
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::free_gc_checked(c, gcontext);
          });
    }

    template<typename C>
    static
    gcontext<Connection, Interfaces ...>
    copy(C && c, xcb_gcontext_t src_gc, uint32_t value_mask)
    {
      return gcontext(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::copy_gc(c, src_gc, gcontext, value_mask);
          },
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::free_gc(c, gcontext);
          });
    }


    template<typename C>
    static
    gcontext<Connection, Interfaces ...>
    copy_checked(C && c, xcb_gcontext_t src_gc, uint32_t value_mask)
    {
      return gcontext(
          std::forward<C>(c),
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::copy_gc_checked(c, src_gc, gcontext, value_mask);
          },
          [&](const Connection & c, const xcb_gcontext_t & gcontext)
          {
            xpp::x::free_gc_checked(c, gcontext);
          });
    }
};

namespace generic {

template<typename Connection, template<typename, typename> class ... Interfaces>
struct traits<xpp::gcontext<Connection, Interfaces ...>>
{
  typedef xcb_gcontext_t type;
};

} // namespace generic

} // namespace xpp

#endif // XPP_GCONTEXT_HPP
