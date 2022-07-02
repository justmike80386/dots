LIBS=x11 \
     xcb \
     xcb-icccm \
     xcb-sync \
     xcb-xf86dri \
     xcb-xprint \
     xcb-xinput \
     xcb-shape \
     xcb-shm \
     xcb-render \
     xcb-proto \
     xcb-event \
     xcb-xfixes \
     xcb-xkb \
     xcb-dri3 \
     xcb-ewmh \
     xcb-util \
     xcb-renderutil \
     xcb-xtest \
     xcb-xevie \
     xcb-keysyms \
     xcb-image \
     xcb-composite \
     xcb-randr \
     xcb-present \
     xcb-xv \
     xcb-aux \
     xcb-record \
     xcb-dpms \
     xcb-glx \
     xcb-atom \
     xcb-damage \
     xcb-screensaver \
     xcb-xvmc \
     xcb-res \
     xcb-xinerama \
     xcb-dri2

CXXFLAGS=-std=c++11 -Wall -O0 $(shell pkg-config --cflags ${LIBS})
LDFLAGS=$(shell pkg-config --libs ${LIBS})
