from utils import _n, _ext, _n_item, get_namespace

_templates = {}

_templates['void_cookie_function'] = \
'''\
%s\
void
%s_checked(Connection && c%s)
{%s\
  xpp::generic::check<Connection, xpp::%s::error::dispatcher>(
      std::forward<Connection>(c),
      %s_checked(std::forward<Connection>(c)%s));
}

%s\
void
%s(Connection && c%s)
{%s\
  %s(std::forward<Connection>(c)%s);
}
'''

def _void_cookie_function(ns, name, c_name, template, return_value, protos, calls, initializer):
    if len(template) == 0: template = "template<typename Connection>\n"
    return _templates['void_cookie_function'] % \
            ( template
            , name
            , protos
            , initializer
            , ns
            , c_name
            , calls
            , template
            , name
            , protos
            , initializer
            , c_name
            , calls
            )

_templates['cookie_static_getter'] = \
'''\
%s\
    static
    %s
    cookie(xcb_connection_t * const c%s)
    {%s\
      return base::cookie(c%s);
    }
'''

def _cookie_static_getter(template, return_value, protos, calls, initializer):
    return _templates['cookie_static_getter'] % \
            ( template
            , return_value
            , protos
            , initializer
            , calls
            )

class CppCookie(object):
    def __init__(self, namespace, is_void, name, reply, parameter_list):
        self.namespace = namespace
        self.is_void = is_void
        self.name = name
        self.reply = reply
        self.parameter_list = parameter_list
        self.request_name = _ext(_n_item(self.name[-1]))
        self.c_name = "xcb" \
            + (("_" + get_namespace(namespace)) if namespace.is_ext else "") \
            + "_" + self.request_name

    def comma(self):
        return self.parameter_list.comma()

    def calls(self, sort):
        return self.parameter_list.calls(sort)

    def protos(self, sort, defaults):
        return self.parameter_list.protos(sort, defaults)

    def iterator_template(self, indent="    ", tail="\n"):
        prefix = "template<typename " + ("Connection, typename " if self.is_void else "")
        return indent + prefix \
                + ", typename ".join(self.parameter_list.iterator_templates \
                                   + self.parameter_list.templates) \
                + ">" + tail \
                if len(self.parameter_list.iterator_templates) > 0 \
                else ""

    def iterator_calls(self, sort):
        return self.parameter_list.iterator_calls(sort)

    def iterator_protos(self, sort, defaults):
        return self.parameter_list.iterator_protos(sort, defaults)

    def iterator_initializers(self):
        return self.parameter_list.iterator_initializers()

    def void_functions(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        return_value = "xcb_void_cookie_t"

        return _void_cookie_function(get_namespace(self.namespace),
                                     self.request_name,
                                     self.c_name,
                                     template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def static_reply_methods(self, protos, calls, template="", initializer=[]):
        inits = "" if len(initializer) > 0 else "\n"
        for i in initializer:
            inits += "\n"
            for line in i.split('\n'):
                inits += "      " + line + "\n"

        if self.is_void: return_value = "xcb_void_cookie_t"
        else: return_value = self.c_name + "_cookie_t"

        return _cookie_static_getter(template,
                                     return_value,
                                     self.comma() + protos,
                                     self.comma() + calls,
                                     inits)


    def make_static_getter(self):
        default = self.static_reply_methods(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = self.static_reply_methods(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = \
                self.static_reply_methods(self.iterator_protos(True, True),
                        self.iterator_calls(False), self.iterator_template(),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = \
                self.static_reply_methods(self.protos(True, True), self.calls(False))

        result = ""

        if (self.parameter_list.has_defaults
            or self.parameter_list.is_reordered()
            or self.parameter_list.want_wrap):
            result += default

        if self.parameter_list.is_reordered():
            result += "\n" + default_args

        if self.parameter_list.want_wrap:
            result += "\n" + wrapped

        return result

    def make_void_functions(self):
        default = self.void_functions(self.protos(False, False), self.calls(False))

        if self.parameter_list.has_defaults:
            default = self.void_functions(self.protos(True, True), self.calls(False))

        wrapped = ""
        if self.parameter_list.want_wrap:
            wrapped = \
                self.void_functions(self.iterator_protos(True, True),
                        self.iterator_calls(False),
                        self.iterator_template(indent=""),
                        self.iterator_initializers())

        default_args = ""
        if self.parameter_list.is_reordered():
            default_args = \
                self.void_functions(self.protos(True, True), self.calls(False))

        result = ""

        if (self.parameter_list.has_defaults
            or self.parameter_list.is_reordered()
            or self.parameter_list.want_wrap):
            result += default

        if self.parameter_list.is_reordered():
            result += "\n" + default_args

        if self.parameter_list.want_wrap:
            result += "\n" + wrapped

        return result
