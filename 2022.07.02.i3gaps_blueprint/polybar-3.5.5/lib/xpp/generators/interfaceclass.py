# vim: set ts=4 sws=4 sw=4:

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext

from cppevent import event_dispatcher_class
from cpperror import error_dispatcher_class

_templates = {}

_templates['interface_class'] = \
"""\
template<typename Derived, typename Connection>
class interface
{
  protected:
    Connection
    connection(void) const
    {
      return static_cast<const Derived *>(this)->connection();
    }

  public:
%s\

    virtual ~interface(void) {}

    const interface<Derived, Connection> &
    %s(void)
    {
      return *this;
    }

%s\
}; // class interface
"""

_ignore_events = \
        { "XCB_PRESENT_GENERIC" }

########## INTERFACECLASS ##########

class InterfaceClass(object):
    def __init__(self):
        self.requests = []
        self.events = []
        self.errors = []

    def add(self, request):
        self.requests.append(request)

    def add_event(self, event):
        if event.opcode_name not in _ignore_events:
            self.events.append(event)

    def add_error(self, error):
        self.errors.append(error)

    def set_namespace(self, namespace):
        self.namespace = namespace

    def make_proto(self):
        ns = get_namespace(self.namespace)
        methods = ""
        for request in self.requests:
            methods += request.make_object_class_inline(True) + "\n\n"

        typedef = []
        if self.namespace.is_ext:
            typedef = [ "typedef xpp::%s::extension extension;" % ns ]

        if len(typedef) > 0:
            typedef = "".join(["    " + s for s in typedef]) + "\n\n"
        else:
            typedef = ""


        return (_templates['interface_class'] \
            % (typedef, ns, methods)) + \
              '\n' + event_dispatcher_class(self.namespace, self.events) + \
              '\n' + error_dispatcher_class(self.namespace, self.errors)

########## INTERFACECLASS ##########
