#include <climits>
#include <unistd.h>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

int main(int argc, char ** argv)
{
  Display * dpy = XOpenDisplay(NULL);
  Window root = DefaultRootWindow(dpy);

  XRRScreenConfiguration * screen_cfg = XRRGetScreenInfo(dpy, root);
  std::cerr << "rate: " << XRRConfigCurrentRate(screen_cfg) << std::endl;

  for (int nsizes = 0; nsizes < 16; ++nsizes) {
    int nrates = 0;
    short * rates = XRRConfigRates(screen_cfg, nsizes, &nrates);
    std::cerr << "nrates: " << nrates << std::endl;
    for (int i = 0; i < nrates; ++i) {
      std::cerr << "rate: " << rates[i] << std::endl;
    }
  }

  int nhosts = 0;
  int state = 0;
  XHostAddress * host_addresses = XListHosts(dpy, &nhosts, &state);
  for (int i = 0; i < nhosts; ++i) {
    std::cerr << "address: " << host_addresses[i].address << std::endl;
  }

  return 0;
}
