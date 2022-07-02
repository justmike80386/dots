#ifndef XPP_GENERIC_FACTORY_HPP
#define XPP_GENERIC_FACTORY_HPP

#include <utility> // std::forward

namespace xpp { namespace generic {

namespace factory {

template<typename ReturnType>
class make_object
{
  public:
    template<typename Connection, typename ... Parameter>
    ReturnType
    operator()(Connection &&, Parameter && ... parameter) const
    {
      return ReturnType { std::forward<Parameter>(parameter) ... };
    }
};

template<typename ReturnType>
class make_object_with_member
{
  public:
    template<typename Member, typename Connection, typename ... Parameter>
    ReturnType
    operator()(Connection && c, Member && member, Parameter && ... parameter) const
    {
      return ReturnType { std::forward<Member>(member)
                        , std::forward<Connection>(c)
                        , std::forward<Parameter>(parameter) ...
                        };
    }
};

template<typename ReturnType>
class make_object_with_connection
{
  public:
    template<typename Connection, typename ... Parameter>
    ReturnType
    operator()(Connection && c, Parameter && ... parameter) const
    {
      return ReturnType { std::forward<Connection>(c)
                        , std::forward<Parameter>(parameter) ...
                        };
    }
};

template<typename ReturnType>
class make_fundamental {
  public:
    template<typename Connection, typename Member, typename ... Parameter>
    ReturnType
    operator()(Connection &&, Member && member) const
    {
      return std::forward<Member>(member);
    }
};

template<typename Connection,
         typename MemberType,
         typename ReturnType,
         typename ... Parameter>
class make
  : public std::conditional<
      std::is_constructible<ReturnType, MemberType>::value,
      make_fundamental<ReturnType>,
      typename std::conditional<
        std::is_constructible<ReturnType,
                              MemberType,
                              Connection,
                              Parameter ...>::value,
        make_object_with_member<ReturnType>,
        typename std::conditional<
          std::is_constructible<ReturnType,
                                Connection,
                                MemberType,
                                Parameter ...>::value,
          make_object_with_connection<ReturnType>,
          make_object<ReturnType>
        >::type
      >::type
    >::type
{};

} // namespace factory

} } // xpp::generic

#endif // XPP_GENERIC_FACTORY_HPP
