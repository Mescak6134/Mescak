# Firmware – Phase 1: WLAN + einfache Weboberfläche

Dieser Ordner enthält das PlatformIO-Projekt für die ESP32-Firmware.
Aktueller Stand: WLAN-Ersteinrichtung per Captive Portal + einfache
Status-Weboberfläche (siehe Phase 1 im [PROJEKT.md](../PROJEKT.md)).

## Was diese Version schon kann

- Beim ersten Start (kein gespeichertes WLAN) öffnet der ESP32 ein eigenes
  Setup-WLAN.
- Über eine Weboberfläche im Setup-WLAN kann das Heim-WLAN ausgewählt und
  das Passwort eingegeben werden.
- Die Zugangsdaten werden im internen Flash-Speicher (NVS) gespeichert.
- Danach verbindet sich der ESP32 automatisch mit dem Heim-WLAN und ist
  unter `http://gebetsruf.local` (oder seiner IP-Adresse) erreichbar.
- Klappt die Verbindung beim nächsten Start nicht, öffnet sich automatisch
  wieder das Setup-WLAN.

Noch **nicht** enthalten (kommt in späteren Phasen): Standortauswahl,
Diyanet-Gebetszeiten, Zeitplan, Audio-Wiedergabe.

## Voraussetzungen

- [PlatformIO](https://platformio.org/) — entweder als VS-Code-Extension
  oder als CLI (`pip install platformio`).
- Ein ESP32-Board (Standard-ESP32-WROOM reicht für diese Phase) per USB
  an den PC angeschlossen.

## Bauen & Flashen

```bash
cd firmware
pio run                 # kompilieren
pio run --target upload # auf den ESP32 flashen
pio device monitor      # Serial-Log ansehen (115200 Baud)
```

In VS Code: PlatformIO-Icon-Leiste am unteren Rand verwenden (Häkchen =
Build, Pfeil = Upload, Stecker-Symbol = Monitor).

## Erste Einrichtung testen

1. Nach dem Flashen öffnet der ESP32 das WLAN **`Gebetsruf-Setup`**
   (Passwort: `namaz1234` — steht auch in `src/wifi_setup.cpp`).
2. Mit dem Handy/Laptop mit diesem WLAN verbinden.
3. Ein Browser sollte sich automatisch öffnen (Captive Portal). Falls
   nicht: `http://192.168.4.1` manuell aufrufen.
4. Heim-WLAN aus der Liste wählen, Passwort eingeben, speichern.
5. Der ESP32 startet neu und verbindet sich mit dem Heim-WLAN.
6. Danach im Heim-WLAN `http://gebetsruf.local` (oder die im Serial-Monitor
   angezeigte IP-Adresse) im Browser öffnen — dort erscheint die
   Status-Seite.

## Bekannte Einschränkungen dieser Phase

- Das Setup-WLAN-Passwort ist fest im Code hinterlegt (dokumentiert in
  dieser README), da das Gerät kein Display hat. Für den produktiven
  Einsatz kann das später individualisiert werden.
- Kein HTTPS — das ist für ein rein lokales WLAN ohne Internetzugriff von
  außen bewusst nicht vorgesehen (siehe Punkt 30 im Projektdokument).
