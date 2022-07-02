#include <iostream>

#include "../event.hpp"
#include "../connection.hpp"

using namespace xpp;
using namespace event;

xcb_window_t
get_window(xcb_button_press_event_t * const e)
{
  return e->event;
}

xcb_window_t
get_window(xcb_motion_notify_event_t * const e)
{
  return e->event;
}

namespace test {

class handler : public dispatcher
              , public sink<button::press>
              , public sink<button::release>
              , public sink<motion::notify>
{
  public:
    void handle(const button::press &)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }

    void handle(const button::release & e)
    {
      if (XCB_BUTTON_PRESS == (e->response_type & ~0x80)) {
        std::cerr << __PRETTY_FUNCTION__ << " XCB_BUTTON_PRESS" << std::endl;
      } else {
        std::cerr << __PRETTY_FUNCTION__ << " XCB_BUTTON_RELEASE" << std::endl;
      }
    }

    void handle(const motion::notify &)
    {
      std::cerr << __PRETTY_FUNCTION__ << std::endl;
    }
};

class container : public direct::container {
  public:
    dispatcher * const
      at(const unsigned int & window) const
    {
      return m_dispatcher.at(window);
    }

    std::unordered_map<unsigned int, dispatcher *> m_dispatcher;
};

struct foo {
  void bar(void) { std::cerr << __PRETTY_FUNCTION__ << std::endl; }
};

class foo_container : public any::container<foo> {
  public:
    foo * const at(const unsigned int & window)
    {
      return &m_foos.at(window);
    }

    std::unordered_map<unsigned int, foo> m_foos;
};

class foo_handler : public any::adapter<foo, button::press, 0, get_window> {
  public:
    using adapter::adapter;

    void handle(foo * const f, const button::press & e)
    {
      std::cerr << __PRETTY_FUNCTION__ << " response_type: " << (int)(e->response_type & ~0x80) << std::endl;
      f->bar();
    }
};

}; // namespace test

int main(int argc, char ** argv)
{
  connection c("");
  source source(c);

  auto tree = c.query_tree(c.root());

  test::handler handler;
  test::container container;

  test::foo_container foo_container;
  test::foo_handler foo_handler(source, foo_container);

  for (auto & window : tree.children()) {
    *(c.grab_pointer(false, window,
                     XCB_EVENT_MASK_BUTTON_PRESS
                     | XCB_EVENT_MASK_BUTTON_RELEASE
                     | XCB_EVENT_MASK_POINTER_MOTION,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                     XCB_NONE, XCB_NONE, XCB_TIME_CURRENT_TIME));

    container.m_dispatcher[window] = &handler;
    foo_container.m_foos[window] = test::foo();
  }

  dispatcher * dispatcher[] =
    { new direct::adapter<button::press, 0, get_window>(source, container)
    , new direct::adapter<button::release, 0, get_window>(source, container)
    , new direct::adapter<motion::notify, 0, get_window>(source, container)
    };

  source.run();

  for (auto * d : dispatcher) {
    delete d;
  }

  return EXIT_SUCCESS;
}
