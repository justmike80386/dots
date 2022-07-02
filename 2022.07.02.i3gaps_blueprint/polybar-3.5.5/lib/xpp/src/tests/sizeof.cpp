// compile with `g++ -std=c++11 test.cpp`
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace test1 {

template<typename T>
struct interface {
  static std::size_t size_of(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    return sizeof(T);
  }
};

template<>
struct interface<void> {
  static std::size_t size_of(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    return sizeof(char);
  }
};

template<typename T>
struct A : public interface<T> {
  A(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " size_of(): "
              << this->size_of() << std::endl;
  }

  static std::size_t size_of(void)
  {
    std::cerr << "YOLOLOL" << std::endl;
    return 0;
  }
};

// template<typename T>
struct B : public interface<int> {
  B(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " size_of(): "
              << this->size_of() << std::endl;
  }
};

// template<typename T>
struct C : public interface<double> {
  C(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " size_of(): "
              << this->size_of() << std::endl;
  }
};

int main(int argc, char ** argv)
{
  test1::A<int> t1_1;
  std::cerr << std::endl;
  test1::A<double> t1_2;
  std::cerr << std::endl;
  test1::A<char> t1_3;
  std::cerr << std::endl;
  test1::A<uint16_t> t1_4;
  std::cerr << std::endl;
  test1::A<void> t1_5;
  std::cerr << std::endl;

  test1::B t2_1;
  std::cerr << std::endl;

  test1::C t3_1;
  std::cerr << std::endl;

  return 0;
}

};

namespace test2 {

struct interface {
  static void test(void);
};

struct A : public interface {
  static void test(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " (A::test)" << std::endl;
  }
  A(void)
  {
    test();
  }
};

struct B : public interface {
  static void test(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " (B::test)" << std::endl;
  }
  B(void)
  {
    test();
  }
};

void test(const interface & i)
{
  A::test();
  B::test();
  // decltype(i)::test();
}

int main(int argc, char ** argv)
{
  test2::A a;
  test2::B b;
  test2::test(a);
  // test2::test();

  return 0;
}

};

namespace test3 {

template<typename T>
struct Base {
  T t;
};

template<int N>
struct select_type;

template<>
struct select_type<1> {
  Base<int> base;
};

template<>
struct select_type<2> {
  Base<double> base;
};

int main(int argc, char ** argv)
{
  constexpr int i = 0;
  test3::select_type<1> st_1;
  test3::select_type<2> st_2;

  return 0;
}

};

namespace test4 {

template<typename T>
struct i {
  virtual bool f(T) = 0;
};

namespace a {
  struct a : public i<int> {
    bool f(int i)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      return i == m_i;
    }
    int m_i = 0;
  };
};

namespace b {
  struct b : public i<int> {
    bool f(int i)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      return i == m_i;
    }
    int m_i = 1;
  };
};

namespace c {
  struct c : public i<int> {
    bool f(int i)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
      return i == m_i;
    }
    int m_i = 2;
  };
};

template<typename ... IS>
struct z : public IS ... {

  // template<typename ... VS>
  void
  run(int i)
  {
    std::cerr << __PRETTY_FUNCTION__ << " with i = " << i << std::endl;
    f(i, static_cast<IS *>(this) ...);
  }

  template<typename I, typename ... ISS>
  void
  f(int v, I * i, ISS ... is)
  {
    if (! i->f(v)) {
      f(v, is ...);
    } else {
      std::cerr << "We've got a winner!" << std::endl;
    }
  }

  template<typename I>
  void
  f(int v, I * i)
  {
    i->f(v);
  }

};

int main(int argc, char ** argv)
{
  test4::z<test4::a::a, test4::b::b, test4::c::c> z;
  for (auto i : { 0, 1, 2, 3 }) {
    z.run(i);
  }
  return 0;
}

};

namespace test5 {

struct pod_generic {
  int id;
  int m_int;
};

struct pod_int {
  int id = 0;
  int m_int;
};

struct pod_double {
  int id = 1;
  double m_double;
};

struct pod_string {
  int id = 2;
  std::string m_string;
};

template<int OpCode>
struct my_pod_wrapper {
  static const int opcode = OpCode;
};

struct my_pod_int_wrapper : public my_pod_wrapper<0> {
  my_pod_int_wrapper(pod_generic * pg)
    : m_pi((pod_int *)pg) {}
  pod_int * m_pi;
};

struct my_pod_double_wrapper : public my_pod_wrapper<1> {
  my_pod_double_wrapper(pod_generic * pg)
    : m_pd((pod_double *)pg) {}
  pod_double * m_pd;
};

struct my_pod_string_wrapper : public my_pod_wrapper<2> {
  my_pod_string_wrapper(pod_generic * pg)
    : m_ps((pod_string *)pg) {}
  pod_string * m_ps;
};

class dispatcher {
  public:
    virtual ~dispatcher(void) {}
    template<typename E> void dispatch(const E & e);
};

template<typename ... Events>
class sink;

template<typename E>
class sink<E> : virtual public dispatcher {
  public:
    virtual void handle(const E & e) = 0;
};

template<typename Event, typename ... Events>
class sink<Event, Events ...>
  : virtual public sink<Event>
  , virtual public sink<Events> ...
{};

template<typename E>
void dispatcher::dispatch(const E & e)
{
  dynamic_cast<sink<E> *>(this)->handle(e);
}

template<int ExtensionId>
class pod_dispatcher {
  public:
    template<typename Dispatcher>
    bool
    operator()(pod_generic * pg, const Dispatcher & D) const
    {
      // std::cerr << __PRETTY_FUNCTION__ << std::endl;
      std::cerr << "POD_DISPATCHER<" << ExtensionId << ">" << std::endl;
      switch (pg->id) {
        case 0:
          // std::cerr << "dispatch with my_pod_int_wrapper" << std::endl;
          D(my_pod_int_wrapper(pg));
          return true;

        case 1:
          // std::cerr << "dispatch with my_pod_double_wrapper" << std::endl;
          D(my_pod_double_wrapper(pg));
          return true;

        case 2:
          // std::cerr << "dispatch with my_pod_string_wrapper" << std::endl;
          D(my_pod_string_wrapper(pg));
          return true;
      };

      return false;
    }
};

template<>
class pod_dispatcher<1> {
  public:
    template<typename Dispatcher>
    bool
    operator()(pod_generic * pg, const Dispatcher & D) const
    {
      std::cerr << "POD_DISPATCHER<1>" << std::endl;
      switch (pg->id) {
        case 0:
          // std::cerr << "dispatch with my_pod_int_wrapper" << std::endl;
          D(my_pod_int_wrapper(pg));
          return true;

        case 1:
          // std::cerr << "dispatch with my_pod_double_wrapper" << std::endl;
          D(my_pod_double_wrapper(pg));
          return true;

        case 2:
          // std::cerr << "dispatch with my_pod_string_wrapper" << std::endl;
          D(my_pod_string_wrapper(pg));
          return true;
      };

      return false;
    }
};

class a : public sink<my_pod_int_wrapper,
                      my_pod_double_wrapper,
                      my_pod_string_wrapper>
{
  public:
    virtual void handle(const my_pod_int_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
    virtual void handle(const my_pod_double_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
    virtual void handle(const my_pod_string_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

class b : public sink<my_pod_string_wrapper,
                      my_pod_double_wrapper>
{
  public:
    virtual void handle(const my_pod_string_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
    virtual void handle(const my_pod_double_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

class c : public sink<my_pod_double_wrapper>
{
  public:
    virtual void handle(const my_pod_double_wrapper &) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

template<typename ... POD_Dispatcher>
struct handler_registry {

  template<typename D>
  bool
  dispatch(pod_generic * pg) const
  {
    std::cerr << "RecursiveDispatchEnd_EvenBetter" << std::endl;
    return D()(pg, *this);
  }

  template<typename D1, typename D2, typename ... Dispatcher>
  bool
  dispatch(pod_generic * pg) const
  {
    std::cerr << "BigFatDispatch_ManFeelsGood" << std::endl;
    D1()(pg, *this);
    return dispatch<D2, Dispatcher ...>(pg);
  }

  bool
  dispatch(pod_generic * pg) const
  {
    std::cerr << "InitialDispatch_JustAbitWeiry" << std::endl;
    return dispatch<POD_Dispatcher ...>(pg);
  }

  template<typename Event>
  void
  operator()(const Event & e) const
  {
    try {
      for (auto & item : m_dispatcher.at(Event::opcode)) {
        item.second->dispatch(e);
      }
    } catch (...) {}
  }

  template<typename Event1, typename Event2, typename ... Events>
  void
  attach(sink<Event1, Event2, Events ...> * s)
  {
    attach(Event1::opcode, s);
    attach((sink<Event2, Events ...> *)s);
  }

  template<typename Event>
  void
  attach(sink<Event> * s)
  {
    attach(Event::opcode, s);
  }

  void attach(unsigned int opcode, dispatcher * d)
  {
    m_dispatcher[opcode].emplace(0, d);
  }

  std::unordered_map<unsigned int,
                     std::multimap<unsigned int, dispatcher *>> m_dispatcher;
};

template<typename I>
void foo(int i, I j) {}

int main(int argc, char ** argv)
{
  int i = 42;
  std::string s = "42";

  pod_int pi;
  pod_double pd;
  pod_string ps;

  test5::a a;
  test5::b b;
  test5::c c;

  handler_registry<pod_dispatcher<0>,
                   pod_dispatcher<1>,
                   pod_dispatcher<2>> registry;

  registry.attach(&a);
  registry.attach(&b);
  registry.attach(&c);

  registry.dispatch((test5::pod_generic *)&pi);
  registry.dispatch((test5::pod_generic *)&ps);
  registry.dispatch((test5::pod_generic *)&pd);

  return 0;
}

};

namespace test6 {
  template <typename... Types>
    struct foo {};

  template < typename... Types1, template <typename...> class T
    , typename... Types2, template <typename...> class V
    , typename U >
    void
    bar(const T<Types1...>&, const V<Types2...>&, const U& u)
    {
      std::cout << sizeof...(Types1) << std::endl;
      std::cout << sizeof...(Types2) << std::endl;
      std::cout << u << std::endl;
    }

  int main(int argc, char ** argv)
    {
      foo<char, int, float> f1;
      foo<char, int> f2;
      bar(f1, f2, 9);
      return 0;
    }
};

namespace test7 {

static constexpr std::size_t * my_ext_1 = nullptr;
static constexpr std::size_t * my_ext_2 = nullptr;
static constexpr std::size_t * my_ext_3 = nullptr;
static constexpr std::size_t * my_ext_4 = nullptr;

// static const std::size_t id_4_0 = reinterpret_cast<std::size_t>(&my_ext_4 + 0);
// static const std::size_t id_4_1 = reinterpret_cast<std::size_t>(&my_ext_4 + 1);
// static const std::size_t id_4_2 = reinterpret_cast<std::size_t>(&my_ext_4 + 2);

namespace p1 {
template<std::size_t * Id>
struct ext {};

template<>
struct ext<my_ext_1> {
  void call(void) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};
};

namespace p2 {
template<std::size_t * Id>
struct ext {};

template<>
struct ext<my_ext_2> {
  void call(void) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};
};

namespace p3 {
template<std::size_t * Id>
struct ext {};

template<>
struct ext<my_ext_3> {
  void call(void) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};
};

struct proto_1 {
  typedef p1::ext<my_ext_1> ext;
  template<typename Handler, typename Event>
  void dispatch(const Handler & h, const Event & e)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    e.call(static_cast<const typename Event::ext &>(h));
  }
};

struct proto_2 {
  typedef p2::ext<my_ext_2> ext;
  template<typename Handler, typename Event>
  void dispatch(const Handler & h, const Event & e)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    e.call(static_cast<const typename Event::ext &>(h));
  }
};

struct proto_3 {
  typedef p3::ext<my_ext_3> ext;
  template<typename Handler, typename Event>
  void dispatch(const Handler & h, const Event & e)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    e.call(static_cast<const typename Event::ext &>(h));
  }
};

struct event_1 {
  typedef p1::ext<my_ext_1> ext;
  typedef proto_1 proto;
  // void call(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  void call(const ext & e) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; e.call(); }
};

struct event_2 {
  typedef p2::ext<my_ext_2> ext;
  typedef proto_2 proto;
  // void call(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  void call(const ext & e) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; e.call(); }
};

struct event_3 {
  typedef p3::ext<my_ext_3> ext;
  typedef proto_3 proto;
  // void call(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
  void call(const ext & e) const { std::cerr << __PRETTY_FUNCTION__ << std::endl; e.call(); }
};

struct pod_event_1 {
  const int id = 1;
};

struct pod_event_2 {
  const int id = 2;
};

struct pod_event_3 {
  const int id = 3;
};

template<typename ... Protos>
struct proto
  : public Protos ...
  , public Protos::ext ...
{

  template<typename Event>
  void run(const Event & event)
  {
    static_cast<typename Event::proto *>(this)->dispatch(*this, event);
  }

};

int main(int argc, char ** argv)
{
  std::cerr << "my_ext_1: " << my_ext_1 << std::endl;
  std::cerr << "my_ext_2: " << my_ext_2 << std::endl;
  std::cerr << "my_ext_3: " << my_ext_3 << std::endl;

  proto<proto_1, proto_2, proto_3> p;

  event_1 e_1;
  event_2 e_2;
  event_3 e_3;

  p.run(e_1);
  p.run(e_2);
  p.run(e_3);

  return 0;
}

}; // namespace test7

namespace test8 {

struct a {
  void operator()(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct b {
  void operator()(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct c {
  void operator()(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

struct wrap_hull
{
  virtual void do_cool_stuff(void) = 0;
};

template<typename ... Args>
struct wrap
  : public wrap_hull
  , public Args ...
{
  void do_cool_stuff(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

template<typename ... Args>
struct test : public Args ...
{
  void trigger(void)
  {
    run<Args ...>();
  }

  template<typename Arg, typename Next, typename ... Rest>
  void run(void)
  {
    run<Arg>();
    run<Next, Rest ...>();
  }

  template<typename Arg>
  void run(void)
  {
    Arg()();
  }

  template<typename Arg, typename Next, typename ... Rest>
  void
  insert(wrap<Arg, Next, Rest ...> * w)
  {
    insert<Arg, Next, Rest ...>(
        reinterpret_cast<wrap<Arg> *>(w),
        reinterpret_cast<wrap<Next, Rest> *>(w) ...);
  }

  template<typename Arg, typename Next, typename ... Rest>
  void
  insert(wrap<Arg> * w, wrap<Next, Rest ...> * ws ...)
  {
    insert<Arg>(w);
    insert<Next, Rest ...>(
        reinterpret_cast<wrap<Next> *>(w),
        reinterpret_cast<wrap<Rest> *>(w) ...);
  }

  template<typename Arg>
  void
  insert(wrap_hull * wh)
  {
    m_wraps.push_back(wh);
  }

  void call_objects(void)
  {
    for (auto * wh : m_wraps) {
      wh->do_cool_stuff();
    }
  }

  std::vector<wrap_hull *> m_wraps;
};

int main(int argc, char ** argv)
{
  struct test<a, b, c> t;
  t.trigger();

  struct wrap<a, b, c> w;
  t.insert(&w);

  t.call_objects();

  return 0;
}

}; // namespace test8

int main(int argc, char ** argv)
{
  // return test1::main(argc, argv);
  // return test2::main(argc, argv);
  // return test3::main(argc, argv);
  // return test4::main(argc, argv);
  // return test5::main(argc, argv);
  // return test6::main(argc, argv);
  // return test7::main(argc, argv);
  return test8::main(argc, argv);
}
