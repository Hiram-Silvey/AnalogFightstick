// Copyright 2021 Hiram Silvey

#include "decoder.h"

#include <vector>
#include "Arduino.h"
#include <EEPROM.h>

using Platform = hs_profile_Profile_Platform;
using PlatformConfig = hs_profile_Profile_PlatformConfig;
using Layout = hs_profile_Profile_Layout;
using Action = hs_profile_Profile_Layout_Action;
using AnalogAction_ID = hs_profile_Profile_Layout_AnalogAction_ID;
using DigitalAction = hs_profile_Profile_Layout_DigitalAction;

const int kMinAddr = 16;
const int kMasks[8] = {
                       0b1,
                       0b11,
                       0b111,
                       0b1111,
                       0b11111,
                       0b111111,
                       0b1111111,
                       0b11111111,
};
const int kLenActionID = 5;
const int kLenAnalogActionValue = 10;
const int kMinAnalogActionID = hs_profile_Profile_Layout_AnalogAction_ID_R_STICK_Y;

namespace {

  std::vector<PlatformConfig> DecodeHeader(int addr) {
    const byte platform_bitmap = EEPROM.read(addr++);
    std::vector<PlatformConfig> configs;
    for (int platform = _hs_profile_Profile_Platform_MIN;
         platform <= _hs_profile_Profile_Platform_MAX; platform++) {
      if (platform_bitmap & (1 << (8 - platform))) {
        PlatformConfig config;
        config.platform = static_cast<Platform>(platform);
        if (configs.size() % 2 == 0) {
          config.position = EEPROM.read(addr) >> 4;
        } else {
          config.position = EEPROM.read(addr++) & 0xFF;
        }
        // Serial.print("Platform = ");
        // Serial.println(config.platform);
        // Serial.print("Position = ");
        // Serial.println(config.position);
        configs.push_back(config);
      }
    }
    return configs;
  }

  int FetchData(int remaining, int& addr, byte& curr_byte, int& unread) {
    // Serial.print("Fetching remaining ");
    // Serial.print(remaining);
    // Serial.print(" bits from byte ");
    // Serial.print(curr_byte);
    // Serial.print(" with ");
    // Serial.print(unread);
    // Serial.print(" unread bits at address ");
    // Serial.println(addr);
    int data = 0;
    while (remaining > 0) {
      int offset = unread - remaining;
      if (offset >= 0) {
        data |= (curr_byte >> offset) & kMasks[remaining - 1];
      } else {
        data |= (curr_byte << offset * -1) & kMasks[remaining - 1];
      }
      int fetched = unread < remaining ? unread : remaining;
      remaining -= fetched;
      unread -= fetched;
      if (unread == 0) {
        curr_byte = EEPROM.read(addr++);
        unread = 8;
        // Serial.print("Byte exhausted. Fetching remaining ");
        // Serial.print(remaining);
        // Serial.print(" bits from byte ");
        // Serial.print(curr_byte);
        // Serial.print(" with 8 unread bits at address ");
        // Serial.println(addr - 1);
      }
    }
    return data;
  }

  Layout DecodeBody(int addr) {
    // Serial.print("Decoding body at address ");
    // Serial.println(addr);
    Layout layout;
    Action* actions[20] = {
                           &layout.thumb_top,
                           &layout.thumb_middle,
                           &layout.thumb_bottom,
                           &layout.index_top,
                           &layout.index_middle,
                           &layout.middle_top,
                           &layout.middle_middle,
                           &layout.middle_bottom,
                           &layout.ring_top,
                           &layout.ring_middle,
                           &layout.ring_bottom,
                           &layout.pinky_top,
                           &layout.pinky_middle,
                           &layout.pinky_bottom,
                           &layout.left_index_extra,
                           &layout.left_middle_extra,
                           &layout.left_ring_extra,
                           &layout.right_index_extra,
                           &layout.right_middle_extra,
                           &layout.right_ring_extra
    };
    byte curr_byte = EEPROM.read(addr++);
    // Serial.print("First body byte = ");
    // Serial.println(curr_byte);
    int unread = 8;
    for (Action* action : actions) {
      int button_id = FetchData(kLenActionID, addr, curr_byte, unread);
      // Serial.print("Button ID = ");
      // Serial.println(button_id);
      if (button_id >= kMinAnalogActionID) {
        // Serial.println("Analog button detected! Reading value...");
        action->action_type.analog.id = static_cast<AnalogAction_ID>(button_id);
        int button_value = FetchData(kLenAnalogActionValue, addr, curr_byte, unread);
        // Serial.print("Button value = ");
        // Serial.println(button_value);
        action->action_type.analog.value = button_value;
        action->which_action_type = hs_profile_Profile_Layout_Action_analog_tag;
      } else {
        action->action_type.digital = static_cast<DigitalAction>(button_id);
        action->which_action_type = hs_profile_Profile_Layout_Action_digital_tag;
      }
    }
    return layout;
  }

}  // namespace

Layout Decoder::Decode(Platform platform, int position) {
  // Serial.print("Looking for platform ");
  // Serial.print(platform);
  // Serial.print(", position ");
  // Serial.println(position);
  const int encoded_len = (EEPROM.read(kMinAddr) << 8) | EEPROM.read(kMinAddr + 1);
  const int max_addr = kMinAddr + encoded_len + 1;
  // Serial.print("Max address = ");
  // Serial.println(max_addr);

  int curr_addr = kMinAddr + 2;
  while (curr_addr < max_addr) {
    // Serial.print("Curr address = ");
    // Serial.println(curr_addr);
    std::vector<PlatformConfig> configs = DecodeHeader(curr_addr);
    // Serial.println("Header decoded!");
    // Advance past the header.
    curr_addr += configs.size() / 2 + configs.size() % 2 + 1;
    // Serial.print("Curr address after header = ");
    // Serial.println(curr_addr);
    if ([&]{
          for (const auto& config : configs) {
            if (config.platform == platform && config.position == position) {
              return true;
            }
          }
          return false;
        }()) {
      // Serial.println("Found correct profile!");
      break;
    }
    // Serial.println("Profile did not match, advancing to next profile...");
    // Advance to the header of the next profile.
    curr_addr += 13;
  }
  if (curr_addr >= max_addr) {
    // Serial.println("Profile not found!");
    exit(1);
  }

  return DecodeBody(curr_addr);
}
