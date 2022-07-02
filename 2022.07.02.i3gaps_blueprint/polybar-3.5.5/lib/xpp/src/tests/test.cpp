#include <climits>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <chrono>

#include <list>

template<typename T>
struct is_callable {
private:
    typedef char(&yes)[1];
    typedef char(&no)[2];

    struct Dummy {};
    struct Fallback { void operator()(); };
    // struct Derived : T, Fallback { };
    struct Derived : std::conditional<! std::is_fundamental<T>::value,
                                      T,
                                      Dummy>::type,
                     Fallback { };

    template<typename U, U> struct Check;

    template<typename>
    static yes test(...);

    template<typename C>
    static no test(Check<void (Fallback::*)(), &C::operator()>*);

public:
    static const bool value = sizeof(test<Derived>(0)) == sizeof(yes);
};


// #include <X11/Xlib.h>
// #include <X11/keysymdef.h>
// #include <X11/extensions/Xrandr.h>
// #include <X11/cursorfont.h> // XC_cross

// #include <xcb/xcbext.h>

// #include "../event.hpp"
// #include "../core/value_iterator.hpp"
// #include "../core/connection.hpp"

// template<typename Iterator>
// void
// test(Iterator begin, Iterator end)
// {
//   std::cerr << "before initializer:";
//   for (auto it = begin; it != end; ++it) {
//     std::cerr << " " << *it;
//   }
//   std::cerr << std::endl;
// 
//   // auto vector = { begin, end };
//   // std::vector<typename value_trait<Iterator>::value_type> vector(begin, end);
// 
//   std::vector<typename value_type<Iterator,
//                                   ! std::is_pointer<Iterator>::value
//                                  >::type>
//                                     vector(begin, end);
// 
//   // std::initializer_list<Iterator> vector = { begin, end };
// 
//   std::cerr << "after initializer (size: " << vector.size() << "):";
//   for (auto & v : vector) {
//     std::cerr << " " << v;
//   }
//   std::cerr << std::endl;
// }

struct foo {
  static void create(int * c, unsigned int xid)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }

  static void destroy(int * c, unsigned int xid)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }
};

struct bar {
  static void create(int * c, unsigned int xid, double d)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }
  static void destroy(int * c, unsigned int xid)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  }
};

template<typename Xid, typename ... Parameters>
struct allocator {

  template<void (*Allocate)(int *, Xid, Parameters ...)>
  struct allocate
  {
    void
    operator()(int * c, Xid xid, Parameters ... parameters)
    {
      Allocate(c, xid, parameters ...);
    }
  };

  template<void (*Deallocate)(int *, Xid)>
  struct deallocate
  {
    void
    operator()(int * c, Xid xid)
    {
      Deallocate(c, xid);
    }
  };

};

template<typename Xid, typename Allocate = void, typename Deallocate = void>
struct xid {

  template<typename ... Parameters>
  xid(Parameters ... parameters)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    m_xid = std::shared_ptr<Xid>(new Xid(0), // xcb_generate_id(c)),
                                 [&](Xid * xid)
                                 {
                                   Deallocate()(NULL, *xid);
                                   delete xid;
                                 });
    Allocate()(NULL, 0, parameters ...);
  }

  std::shared_ptr<Xid> m_xid;
};

template<typename Xid>
struct xid<Xid, void, void>
{
  Xid xid;
};

template<typename Signature1, Signature1 & S1, typename Signature2, Signature2 & S2>
struct wrapper;

template<typename Return1,
         typename ... Args1,
         Return1(&F1)(Args1 ...),
         typename Return2,
         typename ... Args2,
         Return2(&F2)(Args2 ...)>
struct wrapper<Return1(Args1 ...), F1, Return2(Args2 ...), F2> {
  static
  void
  allocate(Args1 ... args)
  {
    F1(args ...);
  }
  static
  void
  deallocate(Args2 ... args)
  {
    F2(args ...);
  }
};

struct caller {
  template<typename Callee>
  void operator()(const Callee & callee, int i)
  {
    switch (i) {
      case 0: callee(42);
        break;
      case 1: callee(2.73);
        break;
      case 2: callee(std::string("foo"));
        break;
    };
  }
};

struct callee {
  void operator()(int i) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  template<typename Arg>
  void operator()(const Arg &) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

namespace mylib {

namespace request {
  enum { checked, unchecked };
};

namespace extension {

using mylib::request::checked;
using mylib::request::unchecked;

namespace request {
template<int RequestType = unchecked>
struct void_request {
  void operator()(void) const noexcept(RequestType == unchecked)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  };
};
};

// template<>
// struct void_request<checked> {
//   void operator()(void) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; };
// };

namespace request {
template<int RequestType = checked>
struct reply_request {
  void operator()(void) const noexcept(RequestType == unchecked)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
  };
};
};

// template<>
// struct reply_request<unchecked> {
//   void operator()(void) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; };
// };

struct interface {
  template<int RequestType = unchecked>
  void void_request(void) const { request::void_request<RequestType>()(); }
  template<int RequestType = checked>
  void reply_request(void) const { request::reply_request<RequestType>()(); }
};

}; // extension

struct c : public extension::interface
{};

}; // mylib

namespace test00 {

enum { checked, unchecked };

template<std::size_t check = unchecked>
struct template_struct {
  template<typename Connection, typename ... Parameter>
  template_struct(Connection c, Parameter ... parameter) {}
};

template_struct<> ts(nullptr);

template<std::size_t check = unchecked, typename Connection, typename ... Parameter>
void
template_function(Connection c, Parameter ... parameter)
{
  if (check == checked) {
    std::cerr << "if (check) { // check == checked" << std::endl;
  } else {
    std::cerr << "} else { // check == unchecked" << std::endl;
  }
}

struct an_interface {
  template<typename std::size_t Check = unchecked, typename ... Parameter>
  void
  template_function(Parameter ... parameter)
  {
    test00::template_function<Check>(m_c, parameter ...);
  }
  double m_c;
};

void test(void)
{
//   two t;
//   // t.do_it_void();
//   // t.do_it_void().unchecked();
//   auto reply_1 = t.do_it_reply();
//   auto reply_2 = t.do_it_reply().unchecked();
//   // reply.checked();
//   // auto reply = t.do_it_reply().checked();

  // template_function(t);
  // template_function(t, reply_1, reply_2);
  // template_function<checked>(reply_1, reply_2);
  // template_function<checked>(reply_2);

  an_interface i;
  i.template_function(0);
  i.template_function<checked>(0);
}

};

namespace test01 {

struct a {
  a(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  a(const std::string & string) : m_string(string) {}
  a(const a & other) : m_string(other.m_string) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  a(a && other) : m_string(std::move(other.m_string)) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  ~a(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  std::string m_string = "struct a";
};

struct b {
  // b(const a & a) : m_a(a) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  // template<typename A>
  // b(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  // b(a & a) : m_a(a) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  // b(a && a) : m_a(std::move(a)) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  b(a && a) : m_a(std::move(a)) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  b(const a & a) : m_a(a) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }

  // b(const b & other) : m_a(other.m_a) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  b(b && other) : m_a(std::move(other.m_a)) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }

  ~b(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }

  // const a & m_a;
  a m_a;
};

b
// foo(const a & a)
foo(const a & a)
{
  // return b(std::move(a));
  return b(a);
}

void test(void)
{
  // b bb = foo(a());
  // b bb = foo(std::move(a()));
  a aa("a string on the stack");
  // b b1 = foo(aa);
  std::cerr << "(before move) aa.m_string: " <<  aa.m_string << std::endl;
  b b2 = foo(std::move(aa));
  std::cerr << "(after move) aa.m_string: " <<  aa.m_string << std::endl;
  // std::cerr << "b1.m_a.m_string: " <<  b1.m_a.m_string << std::endl;
  std::cerr << "b2.m_a.m_string: " <<  b2.m_a.m_string << std::endl;
}

};

namespace test02 {

struct a {
  static void foo(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct b : a {
  static void foo(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct c : b {
  static void foo(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

// struct d : a, b {
//   static void foo(void)
//   {
//     std::cerr << __PRETTY_FUNCTION__ << std::endl;
//     a::foo();
//     b::foo();
//   }
// };

void test(void)
{
  a::foo();
  b::foo();
  c::foo();
  // d::foo();
}

};

namespace test03 {

struct connection {};

struct baz {
  int member;
};

struct fro {
  // fro(const int & i)
  // {
  //   std::cerr << __PRETTY_FUNCTION__ << ": " << i << std::endl;
  // }

  fro(const int & i, connection *)
  {
    std::cerr << __PRETTY_FUNCTION__ << ": " << i << std::endl;
  }
};

struct foo {
  foo(const int & i, double d)
  {
    std::cerr << __PRETTY_FUNCTION__ << ": " << i << "; d: " << d << std::endl;
  }

  // foo(int i, connection *, double d)
  // {
  //   std::cerr << __PRETTY_FUNCTION__ << ": " << i << "; d: " << d << std::endl;
  // }
};

namespace reply_member {

template<typename ReturnType>
class get_with_object {
  public:
    template<typename Arg, typename ... Parameter>
    ReturnType
    operator()(Arg & a, connection *, Parameter ... parameter)
    {
      return ReturnType { a, parameter ... };
    }
};

template<typename ReturnType>
class get_with_object_and_connection {
  public:
    template<typename Arg, typename ... Parameter>
    ReturnType
    operator()(const Arg & a, connection * i, Parameter ... parameter)
    {
      return ReturnType { a, i, parameter ... };
    }
};

template<typename ReturnType>
class get_fundamental {
  public:
    template<typename Arg>
    ReturnType
    operator()(const Arg & arg, connection *)
    {
      return ReturnType { arg };
    }
};

template<typename MemberType, typename ReturnType, typename ... Parameter>
class get
  : public std::conditional<
               std::is_constructible<ReturnType, MemberType>::value,
               get_fundamental<ReturnType>,
               typename std::conditional<
                            std::is_constructible<ReturnType,
                                                  MemberType,
                                                  connection *,
                                                  Parameter ...>::value,
                            get_with_object_and_connection<ReturnType>,
                            get_with_object<ReturnType>
                        >::type
           >::type
{};

};

struct c {
  template<typename ReturnType = int, typename ... Parameter>
  ReturnType
  c_get(Parameter ... parameter)
  {
    using get = reply_member::get<decltype(b.member), ReturnType, Parameter ...>;
    return get()(b.member, i, parameter ...);
  }

  baz b;
  connection * i;
};

void
test(void)
{
  c cc;
  cc.b.member = 42;

  auto m_int = cc.c_get();
  std::cerr << "m: " << m_int << std::endl;

  // auto m_fro_i = cc.c_get<fro>();
  // auto m_foo_d = cc.c_get<foo>(3.14);
}

}; // test03

namespace test04 {

struct a {
  a(const std::string & s) : m_s(s) {}
  void operator()(const std::string & s) const
  {
    std::cerr << "a: " << s << "; m_s: " << m_s << std::endl;
  }
  std::string m_s;
};

struct b {
  void operator()(const std::string & s) const
  {
    std::cerr << "b: " << s << std::endl;
  }
};

struct c : a, b {
  c(void) : a("c : a") {}
  // void operator() (const std::string & s) { a::operator()(s); b::operator()(s); }
};

struct d {
  void
  operator()(const std::string & s) const
  {
    std::cerr << "d: " << s << std::endl;
  }
};

template<typename Base, typename Arg>
void
check(const Base & base, const Arg & arg)
{
  base(arg);
}

template<typename Base, typename Arg, typename E>
void
check(const Base & base, const Arg & arg)
{
  static_cast<const E &>(base)(arg);
}

template<typename Base, typename Arg, typename E, typename Next, typename ... Rest>
void
check(const Base & base, const Arg & arg)
{
  check<Base, Arg, E>(base, arg);
  check<Base, Arg, Next, Rest ...>(base, arg);
}

template<typename E, typename ... ES>
struct foo_base {
  foo_base(const E & e)
    : m_e(e)
  {}

  E m_e;

  void
  check_error(const std::string & s)
  {
    check<E, std::string, ES ...>(m_e, s);
  }
};

template<>
struct foo_base<void> {
  foo_base(void)
  {}

  void
  check_error(const std::string & s)
  {
    std::cerr << "foo_base<void>" << std::endl;
  }
};

template<typename E = void, typename ... ES>
struct foo
  : public foo_base<E, ES ...>
{
  typedef foo_base<E, ES ...> base;

  using base::foo_base;

  template<typename ... P2>
  void
  operator()(const std::string & s, P2 ... p2)
  {
    base::check_error(s);
  }

}; // struct foo;

template<typename T>
using decay = typename std::decay<T>::type;

template<typename Condition, typename T = void>
using enable_if =
  typename std::enable_if<Condition::value, T>::type;

template<typename Condition, typename T = void>
using disable_if =
  typename std::enable_if<! Condition::value, T>::type;

// template<typename P, typename ... PS>
// typename std::enable_if<is_callable<decay<P>>::value, void>::type
// foo_function_dispatch(const std::string & s, P && p, PS ... ps)
// {
//   std::cerr << __PRETTY_FUNCTION__ << std::endl;
//   foo_function(p, s, ps ...);
// }

// template<typename P, typename ... PS>
// typename std::enable_if<! is_callable<decay<P>>::value, void>::type
// foo_function_dispatch(const std::string & s, P && p, PS ... ps)
// {
//   std::cerr << __PRETTY_FUNCTION__ << std::endl;
//   foo_function(s, p, ps ...);
// }

template<typename Parameter,
         typename ... Parameters,
         typename = disable_if<is_callable<decay<Parameter>>>>
void
foo_function(const std::string & s, Parameter && p, Parameters ... ps)
{
  foo<>()(s, ps ...);
}

template<typename ... ErrorHandlers,
         typename ErrorHandler,
         typename ... Parameters,
         typename = enable_if<is_callable<decay<ErrorHandler>>>>
void
foo_function(const std::string & s, ErrorHandler && e, Parameters ... ps)
{
  (foo<ErrorHandler, ErrorHandlers ...>(e))(s, ps ...);
}

struct foo_iface {

  foo_iface(void) {}
  foo_iface(const std::string & s) : m_s(s) {}

  template<typename ... ErrorHandlers, typename ... Parameters>
  void
  foo_method(Parameters ... ps)
  {
    foo_function<ErrorHandlers ...>(m_s, ps ...);
  }

  // template<typename P,
  //          typename ... PS,
  //          typename = disable_if<is_callable<decay<P>>>>
  // void
  // foo_method(P && p, PS ... ps)
  // {
  //   foo_function(m_s, p, ps ...);
  // }

  // template<typename ... ErrorHandlers,
  //          typename E,
  //          typename ... PS,
  //          typename = enable_if<is_callable<decay<E>>>>
  // void
  // foo_method(E && e, PS ... ps)
  // {
  //   foo_function<ErrorHandlers ...>(m_s, e, ps ...);
  // }

  std::string m_s = "foo_iface";
};

// template<typename ... ES, typename E, typename ... PS, typename P>
// void
// foo_with_error(const std::string & s, PS ... ps, const E & e)
// {
//   (foo<E, ES ...>(e))(s, ps ...);
//   // foo<std::function<void(const std::string &)>> f([](const std::string & s) { std::cerr << "lambda: " << s << std::endl; });
//   // f(s, ps ...);
// }

struct e {
  int i;
};

void
test(void)
{
  // d dd;
  // (foo<d>(dd))("aaa 42 aaa", 5,6,7);
  // ((foo<c, a, b>(c())))("ccc 42 ccc", 5,6,7);

  foo_function("hhh 42 hhh", a("abcde"), 5,6,7);
  foo_function("jjj 42 jjj", 5,6,7);
  foo_function("kkk 42 kkk", [](const std::string & s){ std::cerr << "lambda: " << s << std::endl; }, 5,6,7);

  foo_iface fi;
  fi.foo_method(5,6,7);
  fi.foo_method(b(), 5,6,7);
  c cc;
  fi.foo_method<a,b>(cc, 5,6,7);

  std::cerr << std::boolalpha << std::endl
            // << "has_member_operator(): " << has_member_operator<b>::value << std::endl
            << "is_callable: " << is_callable<b>::value << std::endl
            << "is_trivial: " << std::is_trivial<b>::value << std::endl
            << "is_pod: " << std::is_pod<b>::value << std::endl
            << "is_standard_layout: " << std::is_standard_layout<b>::value << std::endl
            << "is_fundamental: " << std::is_fundamental<b>::value << std::endl
            << "is_class: " << std::is_class<b>::value << std::endl
            << "is_object: " << std::is_object<b>::value << std::endl
            << std::endl;

  std::cerr << std::boolalpha << std::endl
            << "is_callable: " << is_callable<e>::value << std::endl
            << "is_trivial: " << std::is_trivial<e>::value << std::endl
            << "is_pod: " << std::is_pod<e>::value << std::endl
            << "is_standard_layout: " << std::is_standard_layout<e>::value << std::endl
            << "is_fundamental: " << std::is_fundamental<e>::value << std::endl
            << "is_class: " << std::is_class<e>::value << std::endl
            << "is_object: " << std::is_object<e>::value << std::endl
            << std::endl;

  std::cerr << std::boolalpha << std::endl
            << "is_callable: " << is_callable<int>::value << std::endl
            << std::endl;

  // foo<b>(b(1))("bbb 42 bbb", 5,6,7);
  // (foo<b>(b()))("bbb 42 bbb", 5,6,7);
  // foo<a, b>(c())("ccc 42 ccc", 5,6,7);
  // foo<>()("ddd 42 ddd", 5,6,7);
  // foo<a>()(a("a2"), "eee 42 eee", 5,6,7);
  // (foo<a>(a("a2")))("eee 42 eee", 5,6,7);
}

}; // test04

namespace test05 {

template<typename T>
struct a {
  struct member {
    struct get {
      void operator()(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
    };
  };
};

struct b : a<int> {
  void
  fun(void)
  {
    using get = member::get;
    get()();
  }
};

void
test(void)
{
  b bb;
  bb.fun();
}

}; // test05

namespace test06 {

void foo(int i)
{
  std::cerr << __PRETTY_FUNCTION__ << std::endl;
}

auto &bar = foo;

struct a {
  virtual void foo(void) = 0;
};

struct b : a {
  using a::foo;
  void bar(void) { foo(); }
  // decltype(a::foo) & bar = a::foo;
};

void test(void)
{
  foo(1);
  bar(2);
}

}; // test05

namespace test07 {

class a {
  static const char * foo ;
  static const std::string bar ;
  static constexpr const char * baz = "baz";
};
const char * a::foo = "foo";
const std::string a::bar = "bar";

void
test(void)
{
  // a aa;
}

}; // test05

namespace test08 {

template<typename T>
struct print {
  template<typename X>
  std::ostream &
  operator()(std::ostream & os, X && t)
  {
    return os << "forward: " << std::forward<T>(t);
  }
};

template<typename T>
struct print<T *> {
  std::ostream &
  operator()(std::ostream & os, T * t)
  {
    return os << "pointer: " << *t;
  }
};

template<typename T>
struct a {
  explicit a(T && t)
    : m_t(std::forward<T>(t))
  {
    std::ostream & os = std::cerr;
    os << __PRETTY_FUNCTION__ << " m_t: ";
    print<T>()(os, m_t);
    os << std::endl;
  }

  explicit a(const T & t)
    : m_t(t)
  {
    std::ostream & os = std::cerr;
    os << __PRETTY_FUNCTION__ << " m_t: ";
    print<T>()(os, m_t);
    os << std::endl;
  }

  ~a(void)
  {
    std::ostream & os = std::cerr;
    os << __PRETTY_FUNCTION__ << " m_t: ";
    print<T>()(os, m_t);
    os << std::endl;
  }

  T m_t;
};

void
test(void)
{
  int i = 0;
  double d = 0.0;
  a<int> ai(i);
  a<double> ad(d);
  a<int> aim(std::move(i));
  a<double> adm(std::move(d));
  a<int *> aip(&i);
  a<double *> adp(&d);
}

}; // test05

namespace test09 {

template<typename T, typename U>
struct a {
  explicit a(const T & t, const U & u)
    : m_t(t)
    , m_u(u)
  {}
  explicit a(const T & t)
    : m_t(t)
  {}
  void operator=(const U & u) { m_u = u; }
  void set(const U & u) { m_u = u; }
  T m_t;
  U m_u;
};

int t = 0;

a<int, int>
get_by_value(const int & u)
{
  return a<int, int>{ t, u };
}

a<int, int> reference_return_a(t);

const a<int, int> &
get_by_reference(const int & u)
{
  reference_return_a = u;
  // reference_return_a.set(u);
  return reference_return_a;
}

a<int, int> &&
get_by_move(const int & u)
{
  return std::move(a<int, int>{t, u});
  // return a<int, int>{t, u};
}

void
test(void)
{
  int len = 1000000;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  std::chrono::time_point<std::chrono::high_resolution_clock> stop;

  std::cerr << "get_by_value:" << std::endl;
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < len; ++i) {
    a<int, int> aa = get_by_value(i);
    std::cerr << "\r" << aa.m_u;
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cerr << std::endl << "duration: " << (stop - start).count() << std::endl;

  std::cerr << "get_by_value_rvalue:" << std::endl;
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < len; ++i) {
    a<int, int> && aa = get_by_value(i);
    std::cerr << "\r" << aa.m_u;
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cerr << std::endl << "duration: " << (stop - start).count() << std::endl;

  std::cerr << "get_by_reference:" << std::endl;
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < len; ++i) {
    const a<int, int> & aa = get_by_reference(i);
    std::cerr << "\r" << aa.m_u;
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cerr << std::endl << "duration: " << (stop - start).count() << std::endl;

  // invalid examples
  /*
  std::cerr << "get_by_move_value:" << std::endl;
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < len; ++i) {
    a<int, int> aa = get_by_move(i);
    std::cerr << "\r" << aa.m_u;
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cerr << std::endl << "duration: " << (stop - start).count() << std::endl;

  std::cerr << "get_by_move_rvalue:" << std::endl;
  start = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < len; ++i) {
    a<int, int> && aa = get_by_move(i);
    std::cerr << "\r" << aa.m_u;
  }
  stop = std::chrono::high_resolution_clock::now();
  std::cerr << std::endl << "duration: " << (stop - start).count() << std::endl;
  */
}

}; // test05

namespace test10 {

class a {
  public:
    a(const unsigned int & i) : m_int(std::make_shared<unsigned int>(i)) {}
    operator unsigned int &(void)
    {
      return *m_int;
    }
    std::shared_ptr<unsigned int> m_int;
};

void
test(void)
{
  a aa { 0 };
  std::cerr << *aa.m_int << std::endl;
  static_cast<unsigned int &>(aa) = 42;
  std::cerr << *aa.m_int << std::endl;
}

}; // test05

namespace test11 {

template<typename T>
class a {
  public:
    a(const T & i) {}
    a(const T & i, const unsigned int &) {}
};

void
test(void)
{
  std::cerr << std::boolalpha;
  // std::cerr << std::is_constructible<a, const unsigned int &>::value << std::endl;
  // std::cerr << std::is_constructible<a, unsigned int &>::value << std::endl;
  // std::cerr << std::is_constructible<a, unsigned int>::value << std::endl;
  std::cerr << std::is_constructible<a<unsigned int>, const unsigned int &>::value << std::endl;
  std::cerr << std::is_constructible<a<unsigned int>, unsigned int &>::value << std::endl;
  std::cerr << std::is_constructible<a<unsigned int>, unsigned int>::value << std::endl;
}

}; // test05

namespace test12 {

void
test(void)
{
  char a[2*sizeof(int)];
  uint * b = reinterpret_cast<uint *>(a);
  b[0] = (uint)'a' + ((uint)'b' << 8) + ((uint)'c' << 16) + ((uint)'d' << 24);
  b[1] = (uint)'e' + ((uint)'f' << 8) + ((uint)'g' << 16) + ((uint)'h' << 24);
  std::cerr << b[0] << ": " << a[0] << " " << a[1] << " " << a[2] << " " << a[3] << std::endl;
  std::cerr << b[1] << ": " << a[4] << " " << a[5] << " " << a[6] << " " << a[7] << std::endl;
}

}; // test05

namespace test13 {

template<typename ... Args>
struct a {
};

template<typename ... Types> struct b;

// template<template<typename ...> class T, typename ... Args>
// template<template<typename ... Args> class T> // , typename ... Args>
// template<typename ... Args>
// template<template<typename T<typename ... Args>>>

template<typename ... Args, template<typename ...> class T>
struct b<T<Args ...>> {
  b(const T<Args ...> & t)
    : m_t(t)
  {}
  T<Args ...> m_t;
};

template<typename ... Args, template<typename ...> class T>
struct b<T<Args ...> &>
  : public b<T<Args ...>>
{
  using base = b<T<Args ...>>;
  using base::base;
};

void
test(void)
{
  a<int> aa_1;
  b<a<int>> bb_1(aa_1);
  b<a<int> &> bb_2(aa_1);
}

}; // test05

namespace test15 {

struct i {
  virtual void fun(void) = 0;
};

struct a : i {
  void fun(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct b {
};

template<typename I>
struct with_i : I
{
  virtual
  void run(void)
  {
    std::cerr << "with_i" << std::endl;
    static_cast<i *>(this)->fun();
  }
};

struct without_i
{
  virtual
  void run(void) { std::cerr << "without_i" << std::endl; }
};

template<typename T>
struct i_test
  : std::conditional<std::is_base_of<i, T>::value, with_i<T>, without_i>::type
{};

void
test(void)
{
  i_test<a> a_test;
  a_test.run();
  i_test<b> b_test;
  b_test.run();
}

}; // test05

int main(int argc, char ** argv)
{
  // if (argc != 2) {
  //   std::cerr << "Need one string argument" << std::endl;
  //   return 1;
  // }

  // xid<xcb_window_t> xid_1;

  // xid<xcb_window_t,
  //     allocator<xcb_window_t>::allocate<&foo::create>,
  //     allocator<xcb_window_t>::deallocate<&foo::destroy>>
  //       xid_2;

  // xid<xcb_window_t,
  //     allocator<xcb_window_t, double>::allocate<&bar::create>,
  //     allocator<xcb_window_t>::deallocate<&bar::destroy>>
  //       xid_3(0.0);

  // wrapper<decltype(foo::create), foo::create,
  //         decltype(foo::destroy), foo::destroy>::allocate(NULL, 0);

  // wrapper<decltype(foo::create), foo::create,
  //         decltype(foo::destroy), foo::destroy>::deallocate(NULL, 0);

  // w_1.allocate(NULL, 0);
  // w_1.deallocate(NULL, 0);

  // // xpp::resource class!
  // // >>>>>>>>><<<<<<<<<<<
  // wrapper<decltype(bar::create), bar::create,
  //         decltype(bar::destroy), bar::destroy>::allocate(NULL, 0, 0.0);

  // wrapper<decltype(bar::create), bar::create,
  //         decltype(bar::destroy), bar::destroy>::deallocate(NULL, 0);

  // wrapper<decltype(bar::create), bar::create,
  //         decltype(bar::destroy), bar::destroy> w_2;
  // w_2.allocate(NULL, 0, 0.0);
  // w_2.deallocate(NULL, 0);

  // wrapper<decltype(bar::create), bar::create> w_2;

  // xpp::connection<> connection("");

  // std::map<int, char> char_map;
  // char_map[0] = 'a';
  // char_map[1] = 'b';
  // char_map[2] = 'c';

  // value_iterator<std::map<int, char>::iterator> begin =
  //   value_iterator<std::map<int, char>::iterator>(char_map.begin());
  // value_iterator<std::map<int, char>::iterator> end =
  //   value_iterator<std::map<int, char>::iterator>(char_map.end());

  // std::vector<char> chars_1 = std::vector<char>(begin, end); std::vector<char> chars_2 = { begin, end };
  // const char * legacy_chars = "abc";
  // std::vector<char> chars_3 = { legacy_chars, legacy_chars + 3 };

  // // for (auto it = begin; it != end; ++it) {
  // for (auto & c : chars_1) {
  //   std::cerr << "value: " << c << std::endl;
  // }

  // value_iterator<std::vector<char>::iterator> v_begin =
  //   value_iterator<std::vector<char>::iterator>(chars_1.begin());
  // value_iterator<std::vector<char>::iterator> v_end =
  //   value_iterator<std::vector<char>::iterator>(chars_1.end());

  // std::vector<char> vchar_1(v_begin, v_end);
  // std::vector<char> vchar_2 = { v_begin, v_end };

  // std::string my_string(argv[1]);
  // std::cerr << "test 1" << std::endl;
  // test(my_string.begin(), my_string.end());
  // std::cerr << "test 2" << std::endl;
  // test(my_string.c_str(), my_string.c_str() + my_string.length());

  // auto atom = connection.intern_atom(false, "MY_STRING");

  // xpp::request::x::change_property(connection, XCB_PROP_MODE_REPLACE,
  //     connection.root(), atom->atom, XCB_ATOM_STRING, 8, my_string.length(),
  //     my_string.c_str());

  // connection.change_property(XCB_PROP_MODE_REPLACE, connection.root(),
  //     atom->atom, XCB_ATOM_STRING, 8,
  //     begin, end);

  // connection.change_property(XCB_PROP_MODE_REPLACE, connection.root(),
  //     atom->atom, XCB_ATOM_STRING, 8,
  //     v_begin, v_end);

  // connection.flush();

  // caller()(callee(), 0);
  // caller()(callee(), 1);
  // caller()(callee(), 2);

  // using mylib::request::checked;
  // using mylib::request::unchecked;

  // mylib::extension::request::void_request<>()();
  // mylib::extension::request::void_request<checked>()();

  // mylib::c c;
  // c.void_request<>();
  // c.reply_request<>();

  // test00::test();
  // test01::test();
  // test02::test();
  // test03::test();
  // test04::test();
  // test05::test();
  // test06::test();
  // test07::test();
  // test08::test();
  // test09::test();
  // test10::test();
  // test11::test();
  // test12::test();
  // test13::test();
  // test14::test();
  test15::test();

  return 0;
}
