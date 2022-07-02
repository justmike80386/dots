// compile with `g++ -std=c++11 test.cpp`
#include <iostream>

#define CALLABLE(FUNCTION) callable<decltype(FUNCTION), FUNCTION>

template<typename Signature, Signature & S>
struct callable;

template<typename Return,
         typename ... Args, Return (&Function)(Args ...)>
struct callable<Return(Args ...), Function> {
  Return operator()(Args ... args)
  {
    std::cerr << __PRETTY_FUNCTION__ << std::endl;
    return Function(args ...);
  }
};

template<typename ... Arguments>
class one_size_fits_them_all;

// A generic template
template<typename T, typename U, typename V,
         typename F1, typename F2, typename F3>
class one_size_fits_them_all<T, U, V, F1, F2, F3>
{
  public:
    one_size_fits_them_all(void)
    {
      std::cerr << "generic one_size_fits_them_all" << std::endl
                << __PRETTY_FUNCTION__ << std::endl << std::endl;
      F1()();
      F2()();
      F3()();
      std::cerr << std::endl;
    }
};

// A specialized template
template<typename T, typename Callable>
class one_size_fits_them_all<T, int, int, void, void, Callable>
{
  public:
    one_size_fits_them_all(void)
    {
      std::cerr << "specialized one_size_fits_them_all" << std::endl
                << __PRETTY_FUNCTION__ << std::endl << std::endl;
      Callable()();
      std::cerr << std::endl;
    }
};

void f1(void)
{
  std::cerr << __PRETTY_FUNCTION__ << std::endl << std::endl;
}

void f2(void)
{
  std::cerr << __PRETTY_FUNCTION__ << std::endl << std::endl;
}

void f3(void)
{
  std::cerr << __PRETTY_FUNCTION__ << std::endl << std::endl;
}

template<typename T>
struct interface {
  static std::size_t size_of(void) { return sizeof(T); }
};

template<typename T>
struct test : public interface<T> {
  test(void)
  {
    std::cerr << __PRETTY_FUNCTION__ << " size_of(): " << this->size_of() << std::endl;
  }
};

int main(int argc, char ** argv)
{
  // generic template
  auto generic = one_size_fits_them_all<
    int, int, int, CALLABLE(f1), CALLABLE(f2), CALLABLE(f3)>();

  // specialized template
  auto specialized_int = one_size_fits_them_all<
    int, int, int, void, void, CALLABLE(f1)>();

  // specialized template
  auto specialized_double = one_size_fits_them_all<
    double, int, int, void, void, CALLABLE(f3)>();

  test<int> t1;
  test<double> t2;
  test<char> t3;
  test<uint16_t> t4;

  return 0;
}
