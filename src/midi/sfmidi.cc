#include "sfmidi.hh"

#include <alsa/asoundlib.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>

#include "util/exception.hh"

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

nanoKONTROL2MIDIDevice::nanoKONTROL2MIDIDevice( const MIDIDeviceInfo& device_info )
  : MIDIDevice( CheckSystemCall(
    "open",
    open( ( "/dev/snd/midiC" + to_string( device_info.card ) + "D" + to_string( device_info.device ) ).c_str(),
          O_RDWR ) ) )
{
  if ( device_info.device_name != DEVICE_NAME ) {
    throw runtime_error( "Not a nanoKONTROL2 MIDI device: " + device_info.device_name );
  }

  set_blocking( false );
}

void nanoKONTROL2MIDIDevice::process_exclusive_message()
{
  string msg = move( pending_exclusive_message_ );
  pending_exclusive_message_ = {};

  throw runtime_error( "Unexpected exclusive message." );
}

void nanoKONTROL2MIDIDevice::read_state()
{
  auto B = []( auto x ) { return 0xFF & static_cast<int>( x ); };

  string data( 512, '\0' );

  while ( not eof() ) {
    size_t len = read( static_cast<string_view>( data ) );

    if ( len == 0 ) {
      return;
    }

    if ( B( data[0] ) == 0xF0 or not pending_exclusive_message_.empty() ) { // exclusive message
      pending_exclusive_message_ += data.substr( 0, len );
      if ( pending_exclusive_message_.back() == '\xF7' ) { // EOX
        process_exclusive_message();
      }
    } else if ( B( data[0] ) == 0xB0 && len == 3 ) {
      if ( 0x00 <= B( data[1] ) and B( data[1] ) <= 0x07 ) { // sliders
        state_.groups[B( data[1] )].slider = B( data[2] );
      } else if ( 0x10 <= B( data[1] ) and B( data[1] ) <= 0x17 ) { // knobs
        state_.groups[0x0F & B( data[1] )].knob = B( data[2] );
      } else if ( 0x20 <= B( data[1] ) and B( data[1] ) <= 0x27 ) { // 's' btns
        state_.groups[0x0F & B( data[1] )].s = B( data[2] );
      } else if ( 0x30 <= B( data[1] ) and B( data[1] ) <= 0x37 ) { // 'm' btns
        state_.groups[0x0F & B( data[1] )].m = B( data[2] );
      } else if ( 0x40 <= B( data[1] ) and B( data[1] ) <= 0x47 ) { // 'r' btns
        state_.groups[0x0F & B( data[1] )].r = B( data[2] );
      }
    } else {
      for ( size_t i = 0; i < len; i++ ) {
        cerr << setw( 2 ) << setfill( '0' ) << hex << B( data[i] ) << ' ';
      }
      cerr << endl;
      throw runtime_error( "Unknown message." );
    }
  }
}
}
