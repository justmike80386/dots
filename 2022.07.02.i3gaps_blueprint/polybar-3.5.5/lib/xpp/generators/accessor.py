from resource_classes import _resource_classes

_templates = {}

_templates['iter_fixed'] = \
"""\
xpp::generic::iterator<Connection,
                       %s,
                       SIGNATURE(%s_%s),
                       SIGNATURE(%s_%s_length)>\
"""

_templates['iter_variable'] = \
"""\
xpp::generic::iterator<Connection,
                       %s,
                       SIGNATURE(%s_next),
                       SIGNATURE(%s_sizeof),
                       SIGNATURE(%s_%s_iterator)>\
"""

_templates['list'] = \
"""\
    xpp::generic::list<Connection,
                       %s_reply_t,
                       %s
                      >
    %s(void)
    {
      return xpp::generic::list<Connection,
                                %s_reply_t,
                                %s
                               >(%s);
    }\
"""

_templates['string_accessor'] = \
'''\
    std::string
    %s(void)
    {
      return std::string(%s_%s(this->get().get()),
                         %s_%s_length(this->get().get()));
    }
'''

def _string_accessor(member, c_name):
    return _templates['string_accessor'] % \
            (member, c_name, member, c_name, member)

class Accessor(object):
    def __init__(self, is_fixed=False, is_string=False, is_variable=False, \
                 member="", c_type="", return_type="", iter_name="", c_name=""):

        self.is_fixed = is_fixed
        self.is_string = is_string
        self.is_variable = is_variable

        self.member = member
        self.c_type = c_type
        self.return_type = return_type
        self.iter_name = iter_name
        self.c_name = c_name

        self.object_type = self.c_type.replace("xcb_", "").replace("_t", "").upper()

        if self.c_type == "void":
            self.return_type = "Type"
        elif self.object_type in _resource_classes:
            self.return_type = self.member.capitalize()
        else:
            self.return_type = self.c_type

    def __str__(self):
        if self.is_fixed:
            return self.list(self.iter_fixed())
        elif self.is_variable:
            return self.list(self.iter_variable())
        elif self.is_string:
            return self.string()
        else:
            return ""


    def iter_fixed(self):
        return_type = self.return_type

        return _templates['iter_fixed'] \
                % (return_type,
                   self.c_name, self.member,
                   self.c_name, self.member)


    def iter_variable(self):
        return _templates['iter_variable'] \
                % (self.c_type,
                   self.iter_name,
                   self.iter_name,
                   self.c_name, self.member)


    def list(self, iterator):
        template = "    template<typename Type" if self.c_type == "void" else ""

        # template<typename Children = xcb_window_t>
        if self.object_type in _resource_classes:
            template += ", " if template != "" else "    template<typename "
            template += self.member.capitalize() + " = " + self.c_type

        template += ">\n" if template != "" else ""

        c_tor_params = "this->m_c, this->get()"

        fst_iterator = "\n                       ".join(iterator.split('\n'))
        snd_iterator = "\n                                ".join(iterator.split('\n'))

        return template + _templates['list'] \
                % (self.c_name,
                   fst_iterator,
                   self.member,
                   self.c_name,
                   snd_iterator,
                   c_tor_params)

    def string(self):
        return _string_accessor(self.member, self.c_name)
