#ifndef XPP_GENERIC_ITERATOR_TRAITS_HPP
#define XPP_GENERIC_ITERATOR_TRAITS_HPP

namespace xpp {

namespace generic {

template<typename T>
struct traits
{
  typedef T type;
};

template<typename Object>
struct conversion_type
{
  using type = typename traits<Object>::type;
};

} // namespace generic

}

#endif // XPP_GENERIC_ITERATOR_TRAITS_HPP
