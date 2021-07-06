// Copyright 2021 Hiram Silvey

#include <memory>

#include "pins.h"
#include "controller.h"
#include "ns_controller.h"
#include "pc_controller.h"

std::unique_ptr<Controller> controller;

void setup() {
  Pins::Init();

  std::vector<std::unique_ptr<Controller>> controllers;
  if (digitalRead(kLeftIndexExtra) == LOW) {
    controllers.push_back(std::make_unique<NSController>());
  } else {
    controllers.push_back(std::make_unique<PCController>());
  }

  while(true) {
    for (auto& c : controllers) {
      if (c->Init()) {
        controller = std::move(c);
        return;
      }
    }
    delay(50);
  }
}

void loop() {
  controller->Loop();
}
