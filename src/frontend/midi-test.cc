#include <cstdlib>
#include <iostream>

#include "midi/sfmidi.hh"

using namespace std;

int main()
{
  for ( auto & device : sfmidi::list_devices() ) {
    cout << device.device_name << " " << device.subdevice_name << endl;
  }

  return EXIT_SUCCESS;
}
