// Copyright 2021 Hiram Silvey

#ifndef MOCK_MCU_H_
#define MOCK_MCU_H_

#include "mcu.h"

#include "gmock/gmock.h"

class MockMCU : public MCU {
 public:
  MOCK_METHOD(bool, DigitalReadLow, (uint8_t pin), (const override));
  MOCK_METHOD(int, Constrain, (int amount, int low, int high),
              (const override));
  MOCK_METHOD(void, SerialWrite, (uint8_t val), (const override));
  MOCK_METHOD(void, SerialWrite, (uint8_t* vals, int size), (const override));
  MOCK_METHOD(int, SerialRead, () (const override));
  MOCK_METHOD(int, SerialAvailable, () (const override));
  MOCK_METHOD(unsigned long, Millis, (), (const override));
  MOCK_METHOD(void, JoystickUseManualSend, (), (const override));
  MOCK_METHOD(void, SetJoystickX, (int val), (const override));
  MOCK_METHOD(void, SetJoystickY, (int val), (const override));
  MOCK_METHOD(void, SetJoystickZ, (int val), (const override));
  MOCK_METHOD(void, SetJoystickZRotate, (int val), (const override));
  MOCK_METHOD(void, SetJoystickSliderLeft, (int val), (const override));
  MOCK_METHOD(void, SetJoystickSliderRight, (int val), (const override));
  MOCK_METHOD(void, SetJoystickButton, (uint8_t pin, bool active), (const override));
  MOCK_METHOD(void, SetJoystickHat, (int angle), (const override));
  MOCK_METHOD(void, JoystickSendNow, (), (const override));
  MOCK_METHOD(uint8_t, EEPROMRead, (int addr), (const override));
  MOCK_METHOD(void, EEPROMUpdate, (int addr, uint8_t val), (const override));
  MOCK_METHOD(void, UpdateHallData, (), (override));
  MOCK_METHOD(float, GetHallX, (), (override));
  MOCK_METHOD(float, GetHallY, (), (override));
  MOCK_METHOD(float, GetHallZ, (), (override));
};

#endif  // MOCK_MCU_H_