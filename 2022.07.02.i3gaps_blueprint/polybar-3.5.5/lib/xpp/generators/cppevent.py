import sys # stderr

from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

from resource_classes import _resource_classes

_field_accessor_template_specialization = \
'''\
template<typename Connection>
template<>
%s
%s<Connection>::%s<%s>(void) const
{
  return %s;
}\
'''

_templates = {}

_templates['field_accessor_template'] = \
'''\
    template<typename ReturnType = %s, typename ... Parameter>
    ReturnType
    %s(Parameter && ... parameter) const
    {
      using make = xpp::generic::factory::make<Connection,
                                               decltype((*this)->%s),
                                               ReturnType,
                                               Parameter ...>;
      return make()(this->m_c,
                    (*this)->%s,
                    std::forward<Parameter>(parameter) ...);
    }\
'''

def _field_accessor_template(c_type, method_name, member):
    return _templates['field_accessor_template'] % \
        ( c_type
        , method_name
        , member
        , member
        )

_templates['event_dispatcher_class'] = \
'''\
namespace event {

template<typename Connection>
class dispatcher
{
  public:
%s\
%s\

    template<typename Handler>
    bool
    operator()(Handler%s,
               const std::shared_ptr<xcb_generic_event_t> &%s) const
    {\
%s
      return false;
    }

%s\
}; // class dispatcher

} // namespace event
'''

def _event_dispatcher_class(typedef, ctors, switch, members, has_events):
    return _templates['event_dispatcher_class'] % \
        ( typedef
        , ctors
        , " handler" if has_events else ""
        , " event" if has_events else ""
        , switch if has_events else ""
        , members
        )

def event_dispatcher_class(namespace, cppevents):
    ns = get_namespace(namespace)

    ctor_name = "dispatcher"

    typedef = []
    ctors = []
    members = []

    opcode_switch = "event->response_type & ~0x80"
    typedef = [ "typedef xpp::%s::extension extension;\n" % ns ]

    members = \
        [ "protected:"
        , "  Connection m_c;"
        ]

    ctors = \
        [ "template<typename C>"
        , "%s(C && c)" % ctor_name
        , "  : m_c(std::forward<C>(c))"
        , "{}"
        ]

    # >>> if begin <<<
    if namespace.is_ext:
        # XXX: The xkb extension contains the sub-event in the member pad0
        if ns == "xkb":
            opcode_switch = "event->pad0"
        else:
            opcode_switch = "(event->response_type & ~0x80) - m_first_event"

        members += [ "  uint8_t m_first_event;" ]

        ctors = \
            [ "template<typename C>"
            , "%s(C && c, uint8_t first_event)" % (ctor_name)
            , "  : m_c(std::forward<C>(c))"
            , "  , m_first_event(first_event)"
            , "{}"
            , ""
            , "template<typename C>"
            , "%s(C && c, const xpp::%s::extension & extension)" % (ctor_name, ns)
            , "  : %s(std::forward<C>(c), extension->first_event)" % ctor_name
            , "{}"
            ]

    # >>> if end <<<

    if len(typedef) > 0:
        typedef = "\n".join(["    " + s for s in typedef]) + "\n"
    else:
        typedef = ""

    if len(ctors) > 0:
        ctors = "\n".join([("    " if len(s) > 0 else "") + s for s in ctors]) + "\n"
    else:
        ctors = ""

    if len(members) > 0:
        members = "\n".join(["  " + s for s in members]) + "\n"
    else:
        members = ""

    switch = event_switch_cases(cppevents, opcode_switch, "handler", "event", namespace)

    return _event_dispatcher_class(typedef,
                                   ctors,
                                   switch,
                                   members,
                                   len(cppevents) > 0)

def event_switch_cases(cppevents, arg_switch, arg_handler, arg_event, ns):
    cases = ""
    first_event_arg = ", m_first_event" if ns.is_ext else ""
    templ = [ "        case %s:"
            , "          %s(" % arg_handler + "%s<Connection>" + "(m_c%s, %s));" % (first_event_arg, arg_event)
            , "          return true;"
            , ""
            , ""
            ]

    distinct_events = [[]]
    for e in cppevents:
        done = False
        for l in distinct_events:
            if e in l:
                continue
            else:
                l.append(e)
                done = True
                break

        if not done:
            distinct_events.append([e])
        else:
            continue

    for l in distinct_events:
        cases += "\n      switch (%s) {\n\n" % arg_switch
        for e in l:
            cases += "\n".join(templ) % (e.opcode_name, e.scoped_name())
        cases += "      };\n"

    return cases if len(cppevents) > 0 else ""

########## EVENT ##########

class CppEvent(object):
    def __init__(self, opcode, opcode_name, c_name, namespace, name, fields):
        self.opcode = opcode
        self.opcode_name = opcode_name
        self.c_name = c_name
        self.namespace = namespace
        self.fields = fields

        self.names = list(map(str.lower, _n_item(name[-1], True)))
        self.name = "_".join(self.names)

        self.nssopen = ""
        self.nssclose = ""
        self.scope = []
        for name in self.names[0:-1]:
            if name in _reserved_keywords: name += "_"
            self.nssopen += " namespace %s {" % name
            self.nssclose += " };"
            self.scope.append(name)

    def __cmp__(self, other):
        if self.opcode == other.opcode:
            return 0
        elif self.opcode < other.opcode:
            return -1
        else:
            return 1

    def get_name(self):
        return _reserved_keywords.get(self.name, self.name)


    def scoped_name(self):
        ns = get_namespace(self.namespace)
        return "xpp::" + ns + "::event::" + self.get_name()

    def make_class(self):
        member_accessors = []
        member_accessors_special = []
        for field in self.fields:
            if field.field_type[-1] in _resource_classes:
                template_name = field.field_name.capitalize()
                c_type = field.c_field_type
                method_name = field.field_name.lower()
                if (method_name == self.get_name()
                    or method_name in _reserved_keywords):
                    method_name += "_"
                member = field.c_field_name

                member_accessors.append(_field_accessor_template(c_type, method_name, member))

        ns = get_namespace(self.namespace)

        extension = "xpp::%s::extension" % ns

        ctor = \
            [ "template<typename C>"
            , "%s(C && c," % self.get_name()
            , (" " * len(self.get_name())) + " const std::shared_ptr<xcb_generic_event_t> & event)"
            , "  : base(event)"
            , "  , m_c(std::forward<C>(c))"
            , "{}"
            ]

        m_first_event = ""

        typedef = [ "typedef xpp::%s::extension extension;" % ns ]

        description = \
            [ "static std::string description(void)"
            , "{"
            , "  return std::string(\"%s\");" % self.opcode_name
            , "}"
            ]

        opcode_accessor = \
            [ "static uint8_t opcode(void)"
            , "{"
            , "  return %s;" % self.opcode_name
            , "}"
            ]

        first_event = []

        if self.namespace.is_ext:
            opcode_accessor += \
                [ ""
                , "static uint8_t opcode(uint8_t first_event)"
                , "{"
                , "  return first_event + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::%s::extension & extension)" % ns
                , "{"
                , "  return opcode(extension->first_event);"
                , "}"
                ]

            first_event = \
                [ "uint8_t first_event(void)"
                , "{"
                , "  return m_first_event;"
                , "}"
                ]

            ctor = \
                [ "template<typename C>"
                , "%s(C && c," % self.get_name()
                , (" " * len(self.get_name())) + " uint8_t first_event,"
                , (" " * len(self.get_name())) + " const std::shared_ptr<xcb_generic_event_t> & event)"
                , "  : base(event)"
                , "  , m_c(std::forward<C>(c))"
                , "  , m_first_event(first_event)"
                , "{}"
                ]

            m_first_event = "    const uint8_t m_first_event;\n"

        if len(opcode_accessor) > 0:
            opcode_accessor = "\n".join(["    " + s for s in opcode_accessor]) + "\n"
        else:
            opcode_accessor = ""

        if len(ctor) > 0:
            ctor = "\n".join(["    " + s for s in ctor]) + "\n"
        else:
            ctor = ""

        if len(typedef) > 0:
            typedef = "\n".join(["    " + s for s in typedef]) + "\n\n"
        else:
            typedef = ""

        if len(member_accessors) > 0:
            member_accessors = "\n" + "\n\n".join(member_accessors) + "\n\n"
            member_accessors_special = "\n" + "\n\n".join(member_accessors_special) + "\n\n"
        else:
            member_accessors = ""
            member_accessors_special = ""

        if len(description) > 0:
            description = "\n" + "\n".join(["    " + s for s in description]) + "\n"
        else:
            description = ""

        if len(first_event) > 0:
            first_event = "\n" + "\n".join(["    " + s for s in first_event]) + "\n"
        else:
            first_event = ""

        return \
'''
namespace event {
template<typename Connection>
class %s
  : public xpp::generic::event<%s>
{
  public:
%s\
    typedef xpp::generic::event<%s> base;

%s\

    virtual ~%s(void) {}

%s\
%s\
%s\
%s\
  protected:
    Connection m_c;
%s\
}; // class %s
%s\
} // namespace event
''' % (self.get_name(), # class %s
       self.c_name, # %s>
       typedef,
       self.c_name, # typedef xpp::generic::event<%s>::base;
       ctor,
       self.get_name(), # virtual ~%s(void) {}
       opcode_accessor,
       description,
       first_event,
       member_accessors,
       m_first_event,
       self.get_name(), # // class %s
       member_accessors_special)
