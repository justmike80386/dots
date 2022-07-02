# vim: set ts=4 sws=4 sw=4:

import sys # stderr
import copy # deepcopy

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

class ObjectClass(object):
    def __init__(self, name):
        self.name = name
        self.requests = []

    def add(self, request):
        if (len(request.parameter_list.parameter) > 0
                and request.parameter_list.parameter[0].c_type == self.c_name):
            request_copy = copy.deepcopy(request)
            request_copy.parameter_list.parameter.pop(0)
            request_copy.make_wrapped()
            self.requests.append(request_copy)

    def set_namespace(self, namespace):
        self.namespace = namespace
        name = (get_namespace(namespace) + "_") if namespace.is_ext else ""
        self.c_name = "xcb_%s_t" % (name + self.name.lower())

    def make_inline(self):
        ns = get_namespace(self.namespace)
        name = self.name.lower()
        c_name = self.c_name
        methods = ""

        for request in self.requests:
            methods += request.make_object_class_inline(False, self.name.lower()) + "\n\n"

        if methods == "":
            return ""
        else:
            return \
"""\
template<typename Derived, typename Connection>
class %s
{
  protected:
    Connection
    connection(void) const
    {
      return static_cast<const Derived *>(this)->connection();
    }

    const %s &
    resource(void) const
    {
      return static_cast<const Derived *>(this)->resource();
    }

  public:
    virtual ~%s(void) {}

%s
}; // class %s
""" % (name,   # class %s
       c_name, # %s resource(void) { ... }
       name, # virtual ~%s(void)
       methods,
       name) # }; // class %s
