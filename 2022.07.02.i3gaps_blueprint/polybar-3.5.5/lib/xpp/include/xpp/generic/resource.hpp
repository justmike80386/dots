#ifndef XPP_GENERIC_RESOURCE_HPP
#define XPP_GENERIC_RESOURCE_HPP

#include <iostream> // std::{hex,dec}
#include <memory> // std::shared_ptr
#include "iterator_traits.hpp"

#include <xcb/xcb.h> // xcb_generate_id

namespace xpp {

namespace generic {

namespace detail {

template<typename Connection, typename Resource, typename ResourceId,
         template<typename, typename> class ... Interfaces>
class interfaces
  : public Interfaces<interfaces<Connection, Resource, ResourceId, Interfaces ...>,
                      Connection> ...
{
  public:
    const ResourceId &
    resource(void) const
    {
      return *static_cast<const Resource &>(*this);
    }

    Connection
    connection(void) const
    {
      return static_cast<const Resource &>(*this).connection();
    }
}; // class interfaces

}

template<typename Connection, typename ResourceId,
         template<typename, typename> class ... Interfaces>
class resource
  : public detail::interfaces<Connection,
                              resource<Connection, ResourceId, Interfaces ...>,
                              ResourceId, Interfaces ...>
{
  protected:
    using self = resource<Connection, ResourceId, Interfaces ...>;

    Connection m_c;
    // reference counting for Resource object
    std::shared_ptr<ResourceId> m_resource;

    resource(Connection c)
      : m_c(c)
    {}

    template<typename C, typename Create, typename Destroy>
    static
    self
    make(C && c, Create create, Destroy destroy)
    {
      self resource(std::forward<C>(c));

      auto xid = xcb_generate_id(std::forward<C>(c));

      // class create before instatiating the shared_ptr
      // create might fail and throw an error, hence shared_ptr would hold an
      // invalid xid, causing possibly another exception in destroy()
      // when create() throws, then the shared_ptr will not be created
      create(std::forward<C>(c), xid);

      resource.m_resource =
        std::shared_ptr<ResourceId>(new ResourceId(xid),
          [&](ResourceId * r)
          {
            destroy(resource.m_c, *r);
            delete r;
          });

      return resource;
    }

  public:
    template<typename C>
    resource(C && c, const ResourceId & resource_id)
      : m_c(std::forward<C>(c))
      , m_resource(std::make_shared<ResourceId>(resource_id))
    {}

    resource(const resource<Connection, ResourceId, Interfaces ...> & other)
      : m_c(other.m_c)
      , m_resource(other.m_resource)
    {}

    virtual
    void
    operator=(const resource<Connection, ResourceId, Interfaces ...> & other)
    {
      m_c = other.m_c;
      m_resource = other.m_resource;
    }

    virtual
    void
    operator=(const ResourceId & resource)
    {
      m_resource = std::make_shared<ResourceId>(resource);
    }

    virtual
    const ResourceId &
    operator*(void) const
    {
      return *m_resource;
    }

    virtual
    operator const ResourceId &(void) const
    {
      return *m_resource;
    }

    Connection
    connection(void) const
    {
      return m_c;
    }
}; // class resource

template<typename Connection, typename ResourceId,
         template<typename, typename> class ... Interfaces>
std::ostream &
operator<<(std::ostream & os,
           const resource<Connection, ResourceId, Interfaces ...> & resource)
{
  return os << std::hex << "0x" << *resource << std::dec;
}

} // namespace generic

} // namespace xpp

#endif // XPP_GENERIC_RESOURCE_HPP
