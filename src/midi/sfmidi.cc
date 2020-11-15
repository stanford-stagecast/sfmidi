#include "sfmidi.hh"

#include <alsa/asoundlib.h>
#include <iostream>

using namespace std;

namespace sfmidi {

vector<MIDIDeviceInfo> list_devices()
{
  vector<MIDIDeviceInfo> result;

  int card = -1;

  while ( snd_card_next( &card ) >= 0 and card >= 0 ) {
    snd_ctl_t* ctl = nullptr;
    string card_name = "hw:" + to_string( card );

    if ( snd_ctl_open( &ctl, card_name.c_str(), 0 ) < 0 ) {
      break;
    }

    int device = -1;
    while ( snd_ctl_rawmidi_next_device( ctl, &device ) >= 0 and device >= 0 ) {
      snd_rawmidi_info_t* dev_info;
      int subdevs_in = 0, subdevs_out = 0;

      snd_rawmidi_info_alloca( &dev_info );
      snd_rawmidi_info_set_device( dev_info, device );

      snd_rawmidi_info_set_stream( dev_info, SND_RAWMIDI_STREAM_INPUT );
      if ( snd_ctl_rawmidi_info( ctl, dev_info ) >= 0 ) {
        subdevs_in = snd_rawmidi_info_get_subdevices_count( dev_info );
      }

      snd_rawmidi_info_set_stream( dev_info, SND_RAWMIDI_STREAM_OUTPUT );
      if ( snd_ctl_rawmidi_info( ctl, dev_info ) >= 0 ) {
        subdevs_out = snd_rawmidi_info_get_subdevices_count( dev_info );
      }

      int subdevs = max( subdevs_in, subdevs_out );
      if ( not subdevs ) {
        continue;
      }

      for ( int sub = 0; sub < subdevs; sub++ ) {
        snd_rawmidi_info_set_stream( dev_info,
                                     sub < subdevs_in ? SND_RAWMIDI_STREAM_INPUT : SND_RAWMIDI_STREAM_OUTPUT );

        snd_rawmidi_info_set_subdevice( dev_info, sub );

        if ( snd_ctl_rawmidi_info( ctl, dev_info ) < 0 ) {
          break;
        }

        string dev_name { snd_rawmidi_info_get_name( dev_info ) };
        string subdev_name { snd_rawmidi_info_get_subdevice_name( dev_info ) };

        MIDIDeviceInfo::Direction direction = MIDIDeviceInfo::Direction::None;

        if ( sub < subdevs_in and sub < subdevs_out ) {
          direction = MIDIDeviceInfo::Direction::InputOutput;
        } else if ( sub < subdevs_in ) {
          direction = MIDIDeviceInfo::Direction::Input;
        } else if ( sub < subdevs_out ) {
          direction = MIDIDeviceInfo::Direction::Output;
        }

        result.push_back( { direction, card, device, sub, dev_name, subdev_name } );
      }
    }
  }

  return result;
}

}
