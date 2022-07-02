#ifndef X_VALUEPARAM_HPP
#define X_VALUEPARAM_HPP

#include <map>
#include <vector>

namespace xpp {

class valueparam {
  public:
    valueparam &
    set(const uint32_t & bit, const uint32_t & value)
    {
      m_has_changed = true;
      m_values_map[bit] = value;
      return *this;
    }

    uint32_t
    mask(void)
    {
      return m_mask;
    }

    uint32_t * const
    values(void)
    {
      if (m_has_changed) {
        m_values.clear();
      }

      for (auto & item : m_values_map) {
        m_values.push_back(item.second);
      }

      m_has_changed = false;

      return m_values.data();
    }

  private:
    bool m_has_changed = true;
    uint32_t m_mask = 0;
    std::vector<uint32_t> m_values;
    std::map<uint32_t, uint32_t> m_values_map;
};

} // namespace xpp

#endif // X_VALUEPARAM_HPP
