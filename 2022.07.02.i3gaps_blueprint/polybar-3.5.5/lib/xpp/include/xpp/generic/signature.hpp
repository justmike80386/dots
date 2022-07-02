#ifndef XPP_GENERIC_SIGNATURE_HPP
#define XPP_GENERIC_SIGNATURE_HPP

#define SIGNATURE(NAME) \
  xpp::generic::signature<decltype(NAME), NAME>

namespace xpp
{
  namespace generic
  {
    template<typename Signature, Signature & S>
    class signature;
  }
}

#endif // XPP_GENERIC_SIGNATURE_HPP
