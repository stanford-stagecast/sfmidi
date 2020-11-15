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

  std::string str() const;
};

std::vector<MIDIDeviceInfo> list_devices();

class MIDIDevice : public FileDescriptor
{};

}
