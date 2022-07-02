# vim: set ts=4 sws=4 sw=4:

import sys # stderr

_templates = {}

_templates['initializer'] = \
'''\
typedef typename value_type<%s, ! std::is_pointer<%s>::value>::type
          vector_type;
std::vector<vector_type> %s =
  { value_iterator<%s>(%s), value_iterator<%s>(%s) };
'''

def _initializer(iter_type, c_name, iter_begin, iter_end):
    return _templates['initializer'] % \
            ( iter_type
            , iter_type
            , c_name
            , iter_type
            , iter_begin
            , iter_type
            , iter_end
            )

class ParameterList(object):
    def __init__(self):
        self.want_wrap = False
        self.has_defaults = False
        self.parameter = []
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.templates = []
        self.iterator_templates = []
        self.initializer = []

    def add(self, param):
        self.has_defaults = param.default != None
        self.parameter.append(param)

    def comma(self):
        return "" if len(self.parameter) == 0 else ", "

    def is_reordered(self):
        tmp = sorted(self.parameter, key=lambda p: p.default or '')
        return tmp != self.parameter

    def calls(self, sort, params=None):
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, key=lambda p: p.default or '')
            ps = tmp
        calls = [p.call() for p in ps]
        return ", ".join(calls)

    def protos(self, sort, defaults, params=None):
        if defaults: sort = True
        ps = self.parameter if params == None else params
        if sort:
            tmp = sorted(ps, key=lambda p: p.default or '')
            ps = tmp
        protos = [p.proto(defaults) for p in ps]
        return ", ".join(protos)

    def iterator_initializers(self):
        return self.initializer

    def make_wrapped(self):
        self.wrap_calls = []
        self.wrap_protos = []
        self.iter_calls = []
        self.iter_2nd_lvl_calls = []
        self.iter_protos = []
        self.initializer = []
        self.templates = []
        self.iterator_templates = []

        lenfields = {}
        # if a parameter is removed, take reduced parameter size into account
        adjust = 0
        for index, param in enumerate(self.parameter):
            prev = index - adjust - 1

            if param.field.type.is_list:
                name = param.field.type.expr.lenfield_name
                if name in lenfields:
                    lenfields[name].append(param.c_name)
                else:
                    lenfields[name] = [ param.c_name ]

                # sys.stderr.write("list: %s %s\n\n"
                #         % ( param.field.type.expr.lenfield_type
                #           , param.field.type.expr.lenfield_name
                #           ))

            # SetGamma: takes 1 size, but 3 value lists
            # if param.field.type.is_list and prev >= 0:
            if (param.is_const and param.is_pointer
                    and prev >= 0
                    and self.parameter[prev].c_name == param.c_name + "_len"):

                adjust = adjust + 1
                self.want_wrap = True
                self.wrap_calls.pop(prev)
                self.wrap_protos.pop(prev)
                self.iter_calls.pop(prev)
                self.iter_2nd_lvl_calls.pop(prev)
                self.iter_protos.pop(prev)

                prev_type = self.parameter[prev].c_type
                if param.c_type == 'char':

                    def append_proto_string(list):
                        list.append(Parameter(None, \
                            c_type='const std::string &',
                            c_name=param.c_name))

                    def append_call_string(list):
                        list.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.length())'))

                        list.append(Parameter(None, \
                            c_name=param.c_name + '.c_str()'))

                    append_proto_string(self.wrap_protos)
                    append_proto_string(self.iter_protos)
                    append_call_string(self.wrap_calls)
                    append_call_string(self.iter_calls)
                    append_call_string(self.iter_2nd_lvl_calls)

                else:
                    param_type = param.c_type
                    if param_type == "void":
                        param_type = "Type_" + str(index)
                        self.templates.append(param_type)

                    prev_type = self.parameter[prev].c_type

                    ### std::vector
                    self.wrap_protos.append(Parameter(None, \
                        c_type='const std::vector<' + param_type + '> &',
                        c_name=param.c_name))

                    self.wrap_calls.append(Parameter(None, \
                      c_name="static_cast<" + prev_type + ">(" \
                      + param.c_name + '.size())'))

                    self.wrap_calls.append(Parameter(None, \
                        c_name=param.c_name + '.data()'))

                    ### Iterator
                    iter_type = param.c_name.capitalize() + "_Iterator"
                    iter_begin = param.c_name + "_begin"
                    iter_end = param.c_name + "_end"

                    if len(self.templates) > 0:
                        self.templates[-1] += " = typename " + iter_type + "::value_type"
                    self.iterator_templates.append(iter_type)

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_begin))

                    self.iter_protos.append(Parameter(None, \
                            c_type=iter_type,
                            c_name=iter_end))

                    self.iter_calls.append(Parameter(None, \
                            c_name="static_cast<" + prev_type + ">(" \
                            + param.c_name + '.size())'))

                    self.iter_calls.append(Parameter(None, \
                            c_name='const_cast<const vector_type *>(' \
                            + param.c_name + '.data())'))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_begin))

                    self.iter_2nd_lvl_calls.append(Parameter(None, \
                            c_name=iter_end))

#                     vector_type = \
#                     '''\
# typename value_type<%s,
#                   ! std::is_pointer<%s>::value
#                  >::type\
#                     ''' % (iter_type, iter_type)

                    # self.initializer.append( \
                    #         "std::vector<%s> %s = { value_iterator<%s>(%s), \
                    #         value_iterator<%s>(%s) };" \
                    #         % (vector_type, param.c_name,
                    #             iter_type, iter_begin,
                    #             iter_type, iter_end))

                    self.initializer.append(
                            _initializer(iter_type, param.c_name, iter_begin, iter_end))

            else:
                self.wrap_calls.append(param)
                self.wrap_protos.append(param)
                self.iter_calls.append(param)
                self.iter_2nd_lvl_calls.append(param)
                self.iter_protos.append(param)

        # end: for index, param in enumerate(self.parameter):

        for k, v in list(lenfields.items()):
            if len(v) > 1:
                sys.stderr.write("list: %s, %s\n" % (k, v))


    def wrapped_calls(self, sort):
        return self.calls(sort, params=self.wrap_calls)

    def wrapped_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.wrap_protos)

    def iterator_calls(self, sort):
        return self.calls(sort, params=self.iter_calls)

    def iterator_2nd_lvl_calls(self, sort):
        return self.calls(sort, params=self.iter_2nd_lvl_calls)

    def iterator_protos(self, sort, defaults):
        return self.protos(sort, defaults, params=self.iter_protos)



_default_parameter_values = \
    { "xcb_timestamp_t" : "XCB_TIME_CURRENT_TIME" }

class Parameter(object):
    def __init__(self, field, c_type="", c_name="", verbose=False):
        self.field = field
        if field != None:
            self.c_type = field.c_field_type
            self.c_name = field.c_field_name
            self.is_const = field.c_field_const_type == "const " + field.c_field_type
            self.is_pointer = field.c_pointer != " "
            # self.serialize = field.type.need_serialize
            self.default = _default_parameter_values.get(self.c_type)
            self.with_default = True
            if verbose:
                sys.stderr.write("c_type: %s; c_name: %s; default: %s\n" \
                      % (self.c_type, self.c_name, self.default))

        else:
            self.c_type = c_type
            self.c_name = c_name
            self.is_const = False
            self.is_pointer = False
            # self.serialize = field.type.need_serialize
            self.default = _default_parameter_values.get(self.c_type)
            self.with_default = True

    def call(self):
        return self.c_name

    def proto(self, with_default):
        c_type = ("const " if self.is_const else "") \
             + self.c_type \
             + (" *" if self.is_pointer else "")
        param = " = " + self.default if with_default and self.default != None else ""
        return c_type + " " + self.c_name + param
