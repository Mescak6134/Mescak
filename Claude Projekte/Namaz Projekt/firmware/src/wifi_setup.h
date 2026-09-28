#pragma once

// Kuemmert sich um WLAN-Ersteinrichtung (Captive Portal) und, sobald
// verbunden, um den einfachen Status-Webserver. Alles Weitere (Standort,
// Gebetszeiten, Audio, ...) kommt in spaeteren Ausbaustufen dazu.
namespace WifiSetup {

// Einmalig in setup() aufrufen.
void begin();

// In jedem loop()-Durchlauf aufrufen.
void loop();

// true, solange sich das Geraet im Setup-Access-Point befindet.
bool isApMode();

} // namespace WifiSetup
