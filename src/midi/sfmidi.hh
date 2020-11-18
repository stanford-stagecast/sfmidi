#pragma once

#include <string>
#include <vector>

#include "util/file_descriptor.hh"

namespace sfmidi {

struct MIDIDeviceInfo
{
  enum class Direction
  {
    None,
    Input,
    Output,
    InputOutput
  };

  const Direction direction;
  const int card;
  const int device;
  const int subdevice;
  const std::string device_name;
  const std::string subdevice_name;
};

std::vector<MIDIDeviceInfo> list_devices();

class MIDIDevice : public FileDescriptor
{
public:
  using FileDescriptor::FileDescriptor;
};

class nanoKONTROL2MIDIDevice : public MIDIDevice
{
public:
  struct State
  {
    struct Group
    {
      uint8_t knob;
      uint8_t slider;
      uint8_t s, m, r;
    };

    Group groups[8];
  };

private:
  State state_ {};
  std::string pending_exclusive_message_ {};

  void process_exclusive_message();

public:
  static constexpr const char* DEVICE_NAME = "nanoKONTROL2";
  nanoKONTROL2MIDIDevice( const MIDIDeviceInfo& device_info );

  const State& state() const { return state_; }
  void read_state();
};

}
