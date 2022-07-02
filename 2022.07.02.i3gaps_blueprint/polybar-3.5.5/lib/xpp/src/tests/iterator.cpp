#include <iostream>

#include "../connection.hpp"

class test {
  public:

    template<typename Data>
    class iterator {
      public:
        iterator(Data * const data, std::size_t index)
          : m_data(data), m_index(index)
        {}

        bool operator==(const iterator & other)
        {
          return m_index == other.m_index;
        }

        bool operator!=(const iterator & other)
        {
          return ! (*this == other);
        }

        const Data & operator*(void)
        {
          return m_data[m_index];
        }

        // prefix
        iterator & operator++(void)
        {
          ++m_index;
          return *this;
        }

        // postfix
        iterator operator++(int)
        {
          auto copy = *this;
          ++(*this);
          return copy;
        }

        // prefix
        iterator & operator--(void)
        {
          --m_index;
          return *this;
        }

        // postfix
        iterator operator--(int)
        {
          auto copy = *this;
          --(*this);
          return copy;
        }

      private:
        Data * const m_data;
        std::size_t m_index = 0;
    };


    template<typename T>
    iterator<T> begin(void)
    {
      throw "This must not happen!";
    }

    template<typename T>
    iterator<T> end(void)
    {
      throw "This must not happen!";
    }

  // private:
    std::vector<int> m_ints;
    std::vector<double> m_doubles;

};

template<>
test::iterator<int> test::begin<int>(void)
{
  return iterator<int>(m_ints.data(), 0);
}

template<>
test::iterator<double> test::begin<double>(void)
{
  return iterator<double>(m_doubles.data(), 0);
}

template<>
test::iterator<int> test::end<int>(void)
{
  return iterator<int>(m_ints.data(), m_ints.size());
}

template<>
test::iterator<double> test::end<double>(void)
{
  return iterator<double>(m_doubles.data(), m_ints.size());
}

int main(int argc, char ** argv)
{
  xpp::connection c("");

  auto tree = c.query_tree(c.root());

  std::cerr << "#windows (children_len): " << tree->children_len << std::endl;
  std::cerr << "#windows (length):       " << tree->length << std::endl;

  std::cerr << std::hex;
  for (auto & window : tree.children()) {
    std::cerr << "0x" << window << "; ";
  }
  std::cerr << std::dec << std::endl;;

  std::cerr << std::hex;
  for (auto it = tree.children().begin(); it != tree.children().end(); ++it) {
    std::cerr << "0x" << *it << "; ";
  }
  std::cerr << std::dec << std::endl;;

  std::cerr << std::hex;
  auto it = tree.children().begin();
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "++it: " << *(++it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "it++: " << *(it++) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "++it: " << *(++it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "--it: " << *(--it) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << "it--: " << *(it--) << std::endl;
  std::cerr << "it  : " << *it     << std::endl;
  std::cerr << std::dec << std::endl;;

  auto atom = c.intern_atom(false, "_NET_CLIENT_LIST_STACKING");
  auto properties = c.get_property<xcb_window_t>(
      false, c.root(), atom->atom, XCB_ATOM_WINDOW, 0, UINT32_MAX);

  std::cerr << std::hex;
  for (auto & window : properties) {
    std::cerr << "0x" << window << "; ";
  }
  std::cerr << std::dec << std::endl;;

  return EXIT_SUCCESS;
}
