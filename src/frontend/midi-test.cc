#include <cstdlib>
#include <iostream>
#include <poll.h>

#include "midi/sfmidi.hh"

using namespace std;

int main()
{
  const auto devices = sfmidi::list_devices();

  if ( devices.empty() ) {
    return EXIT_SUCCESS;
  }

  sfmidi::nanoKONTROL2MIDIDevice device { devices[0] };

  pollfd fds[1];
  fds[0].fd = device.fd_num();
  fds[0].events = POLLIN;

  while ( poll( fds, 1, -1 ) >= 0 ) {
    device.read_state();
  }

  return EXIT_SUCCESS;
}
