from utils import \
        get_namespace, \
        get_ext_name, \
        _n_item, \
        _ext, \
        _reserved_keywords

_templates = {}

_templates['error_dispatcher_class'] = \
'''\
namespace error {

class dispatcher
{
  public:
%s\
%s\

    void
    operator()(const std::shared_ptr<xcb_generic_error_t> &%s) const
    {
%s\
    }

%s\
}; // class dispatcher

} // namespace error
'''

def _error_dispatcher_class(typedef, ctors, switch, members, has_errors):
    return _templates['error_dispatcher_class'] % \
        ( typedef
        , ctors
        , " error" if has_errors else ""
        , switch if has_errors else ""
        , members
        )


def error_dispatcher_class(namespace, cpperrors):
    ns = get_namespace(namespace)

    ctor_name = "dispatcher"

    typedef = []
    ctors = []
    members = []
    opcode_switch = "error->error_code"

    typedef = [ "typedef xpp::%s::extension extension;\n" % ns ]

    # >>> if begin <<<
    if namespace.is_ext:
        opcode_switch = "error->error_code - m_first_error"

        members += \
            [ "protected:"
            , "  uint8_t m_first_error;"
            ]

        ctors = \
            [ "%s(uint8_t first_error)" % (ctor_name)
            , "  : m_first_error(first_error)"
            , "{}"
            , ""
            , "%s(const xpp::%s::extension & extension)" % (ctor_name, ns)
            , "  : %s(extension->first_error)" % ctor_name
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

    switch = error_switch_cases(cpperrors, opcode_switch, "error")
    return _error_dispatcher_class(typedef,
                                   ctors,
                                   switch,
                                   members,
                                   len(cpperrors) > 0)

def error_switch_cases(cpperrors, arg_switch, arg_error):
    cases = ""
    errors = cpperrors
    templ = [ "        case %s: // %s"
            , "          throw %s" + "(%s);" % arg_error
            , ""
            , ""
            ]

    cases += "      switch (%s) {\n\n" % arg_switch
    for e in errors:
        cases += "\n".join(templ) % (e.opcode_name, e.opcode, e.scoped_name())
    cases += "      };\n"

    return cases


class CppError(object):
    def __init__(self, error, namespace, name, c_name, opcode, opcode_name):
        self.error = error
        self.namespace = namespace
        self.c_name = c_name
        self.opcode = opcode
        self.opcode_name = opcode_name

        self.names = list(map(str.lower, _n_item(name[-1], True)))
        self.name = "_".join(self.names)

        self.nssopen = ""
        self.nssclose = ""
        self.scope = []
        for name in self.names[0:-1]:
            if name in _reserved_keywords: name += "_"
            self.nssopen += " namespace %s {" % name
            self.nssclose += " }"
            self.scope.append(name)

    def get_name(self):
        return _reserved_keywords.get(self.name, self.name)


    def scoped_name(self):
        ns = get_namespace(self.namespace)
        return "xpp::" + ns + "::error::" + self.get_name()

    def make_class(self):
        ns = get_namespace(self.namespace)
        typedef = []
        members = []

        opcode_accessor = \
            [ "static uint8_t opcode(void)"
            , "{"
            , "  return %s;" % self.opcode_name
            , "}"
            ]

        if self.namespace.is_ext:
            opcode_accessor += \
                [ ""
                , "static uint8_t opcode(uint8_t first_error)"
                , "{"
                , "  return first_error + opcode();"
                , "}"
                , ""
                , "static uint8_t opcode(const xpp::%s::extension & extension)" % ns
                , "{"
                , "  return opcode(extension->first_error);"
                , "}"
                ]

            members = \
                [ "protected:"
                , "  uint8_t m_first_error;"
                ]

        if len(opcode_accessor) > 0:
            opcode_accessor = "\n".join(["    " + s for s in opcode_accessor]) + "\n"
        else:
            opcode_accessor = ""

        if len(members) > 0:
            members = "\n" + "\n".join(["  " + s for s in members]) + "\n"
        else:
            members = ""

        if len(typedef) > 0:
            typedef = "\n".join(["    " + s for s in typedef]) + "\n\n"
        else:
            typedef = ""

        name = self.name
        if self.name in _reserved_keywords: name = self.name + "_"

        return \
'''
namespace error {
class %s
  : public xpp::generic::error<%s,
                               %s>
{
  public:
%s\
    using xpp::generic::error<%s, %s>::error;

    virtual ~%s(void) {}

%s
    static std::string description(void)
    {
      return std::string("%s");
    }
%s\
}; // class %s
} // namespace error
''' % (self.get_name(), # class %s
       self.get_name(), # : public xpp::generic::error<%s,
       self.c_name, # %s>
       typedef,
       self.get_name(), self.c_name, # using xpp::generic::error<%s, %s>::error;
       self.get_name(), # virtual ~%s(void) {}
       opcode_accessor,
       self.opcode_name, # static constexpr const char * opcode_literal
       members,
       self.get_name()) # // class %s
