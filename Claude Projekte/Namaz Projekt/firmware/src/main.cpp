#include <Arduino.h>
#include "wifi_setup.h"

void setup() {
  WifiSetup::begin();
}

void loop() {
  WifiSetup::loop();
}
