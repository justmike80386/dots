include ../../include/flags.makefile

CXXFLAGS+=-g
CXXFLAGS+=-Wextra

CPPSRCS=$(shell find . -name '*.cpp')
EXAMPLES=${CPPSRCS:./%.cpp=%}

all: ${EXAMPLES}

clean:
	rm -f ${EXAMPLES}

.PHONY: clean
