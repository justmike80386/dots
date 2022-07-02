include ../../include/flags.makefile

# CXX=clang
CXXFLAGS+=-g
# CXXFLAGS+=-Wextra
# CXXFLAGS+=-ftime-report

CPPSRCS=event.cpp \
        requests.cpp \
        iterator.cpp

all: ${CPPSRCS}

${CPPSRCS}:
	${CXX} ${LDFLAGS} ${CXXFLAGS} -o $(@:%.cpp=%) $@

xlib-test: xlib-test.cpp
	${CXX} $(shell pkg-config --libs --cflags x11 xrandr) -o $@ $<

version:

clean:
	rm -f ${CPPSRCS:%.cpp=%}

.PHONY: ${CPPSRCS} clean
