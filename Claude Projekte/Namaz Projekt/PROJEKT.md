# Namaz Projekt: ESP32-basierter Ezan-/Gebetsruf-Lautsprecher

## 1. Ziel des Projekts

Es soll ein eigenes, möglichst einfach zu bedienendes Ezan-/Gebetsruf-System für die Oma gebaut werden.

- Das Gerät soll automatisch zu den korrekten Gebetszeiten einen vorinstallierten Ezan/Gebetsruf abspielen.
- Die tägliche Bedienung soll möglichst einfach sein. Deshalb soll es grundsätzlich keine komplizierte Bedienung über Taster oder Display geben. Stattdessen soll der ESP32 eine eigene Weboberfläche bereitstellen, über die praktisch alles eingerichtet und verändert werden kann.
- Das Gerät soll nach der Einrichtung möglichst selbstständig funktionieren.

## 2. Grundidee

Hauptbestandteile:

- ESP32 als Hauptcontroller
- WLAN
- Weboberfläche auf dem ESP32
- Audioausgabe über I2S-Verstärker + kleinen Lautsprecher (siehe Entscheidung in Punkt 13)
- vorinstallierte Ezan-Aufnahmen
- Diyanet-Gebetszeiten als Datenquelle
- automatische Uhrzeitsynchronisierung über das Internet
- automatische Standortauswahl entsprechend der Diyanet-Ortsstruktur
- lokale Speicherung der Gebetszeiten als Fallback
- einstellbare Zeitverschiebungen
- automatische WLAN-Ersteinrichtung
- möglichst zuverlässiger Betrieb auch bei Internetausfall

## 3. Bedienung

Die eigentliche Bedienung soll hauptsächlich über die Weboberfläche erfolgen. Beispielsweise könnte man sich mit dem Handy mit dem ESP32 verbinden und eine lokale Adresse öffnen:

```
http://gebetsruf.local
```

Alternativ könnte die IP-Adresse des ESP32 verwendet werden.

Über die Weboberfläche sollen unter anderem folgende Einstellungen möglich sein:

- WLAN einrichten
- Standort auswählen (Land, Region/Bundesland, Stadt, Diyanet-Standort festlegen)
- Gebetszeiten anzeigen / aktualisieren
- Zeitverschiebungen einstellen
- Lautstärke einstellen
- Audio-Dateien auswählen / hochladen
- Ezan manuell testen / abspielen
- automatische Wiedergabe aktivieren/deaktivieren
- Status des Systems anzeigen (Uhrzeit, WLAN-Status, Internetstatus, Zeitpunkt der letzten Diyanet-Aktualisierung)
- eventuell Akkuinformationen anzeigen
- eventuell spätere Firmware-Updates ermöglichen

## 4. WLAN-Ersteinrichtung

Wenn der ESP32 noch kein WLAN kennt:

1. ESP32 startet.
2. ESP32 erkennt, dass kein WLAN konfiguriert ist.
3. ESP32 erstellt vorübergehend ein eigenes Setup-WLAN.
4. Handy verbindet sich mit diesem WLAN.
5. Eine Einrichtungsseite wird angezeigt.
6. Dort werden verfügbare WLAN-Netzwerke angezeigt.
7. Benutzer wählt das eigene WLAN.
8. WLAN-Passwort wird eingegeben.
9. ESP32 speichert die Zugangsdaten.
10. ESP32 verbindet sich mit dem normalen WLAN.
11. Danach kann die normale Weboberfläche verwendet werden.

Das Setup-WLAN sollte nicht dauerhaft offen bleiben. Für die Einrichtung sollte möglichst eine sichere bzw. temporäre Absicherung verwendet werden.

## 5. Betrieb ohne Internet

Das Gerät soll nicht vollständig vom Internet abhängig sein.

**Wenn Internet vorhanden ist:**
- Uhrzeit synchronisieren
- Diyanet-Gebetszeiten aktualisieren
- Standortdaten aktualisieren bzw. überprüfen
- kommende Gebetszeiten lokal speichern

**Wenn Internet ausfällt:**
- bereits gespeicherte Gebetszeiten weiterhin verwenden
- lokal gespeicherte Uhrzeit weiterverwenden
- Ezan weiterhin automatisch abspielen

**Wenn Internet wieder verfügbar ist:**
- Uhrzeit erneut synchronisieren
- aktuelle Diyanet-Daten abrufen
- lokale Daten aktualisieren

Dadurch soll ein kurzer Internetausfall nicht dazu führen, dass der Ezan nicht mehr abgespielt wird.

## 6. Diyanet als Datenquelle

Es soll geprüft und verwendet werden, was Diyanet offiziell für Gebetszeiten und Ortsdaten bereitstellt.

Es wurde festgestellt, dass Diyanet einen offiziellen „Awqat Salah"-REST-Service dokumentiert, über den unter anderem Länder, Regionen/Bundesländer, Städte und Gebetszeiten abgerufen werden können.

Die Idee ist deshalb, die Standortauswahl möglichst an der Diyanet-Struktur auszurichten:

```
Land → Region/State → Stadt → Diyanet-Ort/ID
```

Dadurch müssen nicht selbst sämtliche Städte und Gebetszeiten berechnet oder gepflegt werden. Die konkrete API-Nutzung muss beim Programmieren anhand der aktuellen Diyanet-Dokumentation geprüft werden, weil sich Schnittstellen ändern können.

### Entscheidung (Stand 28.09.2026): offizielle API vs. freier Spiegel

Es gibt zwei mögliche Datenquellen:

**A) Offizielle API `awqatsalah.diyanet.gov.tr`**
- **Bestätigt (offizielles Antragsformular + Handbuch von Diyanet geprüft, Stand 28.09.2026):** Erfordert ein unterschriebenes „İstek ve Taahhüt Formu" mit Name, **Kimlik-Numarası (türkische Ausweis-/Staatsbürgerschaftsnummer)**, Adresse und Telefonnummer, das per E-Mail an `dinisleriyk@diyanet.gov.tr` geschickt werden muss – keine Selbstregistrierung, klar auf türkische Staatsbürger ausgelegt.
- Login per JWT (Access Token 30 Min. gültig, Refresh Token +15 Min. danach).
- Rate-Limit laut offiziellem Handbuch: „Developer"-Rolle 100 Requests/Tag (nur befristet nach Anmeldung), danach „Standard"-Rolle nur noch **5 Requests pro Endpoint und Tag**.
- **Damit klar: nicht frei nutzbar ohne Antrag, und für unseren Anwendungsfall ohnehin zu restriktiv.**

**B) Freier Spiegel `ezanvakti.emushaf.net`** (Nachfolger von `ezanvakti.herokuapp.com`)
- Spiegelt dieselben offiziellen Diyanet-Daten, **ohne Registrierung, Formular oder API-Key**.
- Wird von zahlreichen bekannten Ezan-Apps produktiv genutzt, läuft seit Jahren stabil.
- Endpunkte passen praktisch 1:1 auf unsere geplante Struktur:
  ```
  GET /ulkeler                  → Länderliste
  GET /sehirler/{ULKE_KODU}     → Städte/Regionen eines Landes
  GET /ilceler/{SEHIR_KODU}     → Bezirke/Orte einer Stadt
  GET /vakitler/{ILCE_KODU}     → Gebetszeiten für einen Ort
  ```

**Entscheidung:** Wir nutzen **Variante B (`ezanvakti.emushaf.net`)** als primäre Datenquelle. Das spart Bürokratie, Token-Refresh-Logik und Rate-Limit-Sorgen. Die offizielle API (Variante A) bleibt als mögliches späteres Upgrade im Hinterkopf, ist aber kein Blocker.

**Risiko:** Der Spiegel-Dienst ist inoffiziell und könnte theoretisch eingestellt werden. Deshalb bleibt der lokale Fallback (siehe Punkt 11) wichtig – auch unabhängig von der Quelle.

## 7. Standort

Ein eigenes GPS-Modul ist zunächst NICHT vorgesehen. Stattdessen soll der Standort über die Diyanet-Ortsauswahl festgelegt werden.

Beispiel:
- Land: Deutschland
- Region: Baden-Württemberg
- Stadt: Stuttgart

Die entsprechende Diyanet-ID bzw. Ortskennung soll gespeichert werden. Optional könnte später zusätzlich eine manuelle Eingabe von Koordinaten oder eine automatische Standortbestimmung ergänzt werden. GPS ist zunächst nicht notwendig.

## 8. Gebetszeiten

Es sollen mindestens diese Gebetszeiten unterstützt werden:

- Fajr
- Dhuhr
- Asr
- Maghrib
- Isha

Optional können weitere Zeiten wie Sonnenaufgang angezeigt werden, auch wenn dafür kein Ezan abgespielt werden soll. Die Zeiten sollen möglichst direkt aus der Diyanet-Datenquelle kommen. Es soll nicht einfach eine selbst erfundene Berechnung verwendet werden, wenn offizielle Diyanet-Daten verfügbar sind.

## 9. Zeitverschiebungen

Für jedes Gebet soll eine eigene Korrektur eingestellt werden können.

Beispiel:
- Fajr: 0 Minuten
- Dhuhr: +2 Minuten
- Asr: 0 Minuten
- Maghrib: +3 Minuten
- Isha: +5 Minuten

Die offizielle Diyanet-Zeit bleibt sichtbar. Die tatsächliche Abspielzeit ergibt sich aus:

```
Diyanet-Zeit + persönliche Korrektur
```

Beispiel: Diyanet Maghrib 19:43, Korrektur +3 Minuten → Ezan 19:46.

Die Korrekturen sollen in der Weboberfläche frei einstellbar sein, möglichst sowohl positive als auch negative Werte.

## 10. Uhrzeit

Die Uhrzeit soll nicht manuell eingestellt werden müssen. Der ESP32 soll die Uhrzeit regelmäßig über NTP bzw. einen zuverlässigen Internet-Zeitdienst synchronisieren.

Dabei sollen berücksichtigt werden:
- richtige Zeitzone
- Sommer-/Winterzeit
- regelmäßige Synchronisation

Die Uhrzeit soll lokal weiterlaufen, wenn Internet vorübergehend ausfällt.

Die Zeit und die Gebetszeiten sollen getrennt behandelt werden:
- Uhrzeit: NTP/Internet-Zeitdienst
- Gebetszeiten: Diyanet

## 11. Lokaler Fallback

Der ESP32 soll mehrere Tage bzw. möglichst einen längeren Zeitraum an Gebetszeiten lokal speichern, z.B.:

```
28.09. – Fajr, Dhuhr, Asr, Maghrib, Isha
29.09. – Fajr, Dhuhr, Asr, Maghrib, Isha
30.09. – ...
```

Damit funktioniert das System auch bei einem Internetausfall. Wenn wieder Internet vorhanden ist, sollen neue Daten geladen und gespeichert werden.

## 12. Audio

Die Ezan-Aufnahmen sollen lokal gespeichert werden. Vorgesehene Möglichkeit: microSD-Karte, z.B.:

```
/azan/fajr.mp3
/azan/dhuhr.mp3
/azan/asr.mp3
/azan/maghrib.mp3
/azan/isha.mp3
```

Die Weboberfläche soll eventuell ermöglichen:
- Audio-Dateien hochzuladen
- Dateien zu löschen
- Dateien umzubenennen
- Datei einem Gebet zuzuweisen
- Audio vorab zu testen

Es könnten unterschiedliche Aufnahmen für unterschiedliche Gebetszeiten verwendet werden.

## 13. Lautsprecher

Ursprünglich war geplant, den vorhandenen **Anker Soundcore 2 Bluetooth-Lautsprecher** über AUX anzusteuern (siehe „Verworfene Idee" unten).

### Entscheidung (Stand 28.09.2026): eigener Mini-Lautsprecher statt Soundcore 2

Da für den Ezan keine Hi-Fi-Qualität nötig ist, sondern nur gute Hörbarkeit, verwenden wir stattdessen einen **eigenen kleinen Lautsprecher**, direkt am ESP32 über einen **I2S-Verstärker (MAX98357A)** angeschlossen:

```
ESP32 (I2S) → MAX98357A-Verstärkermodul → kleiner Lautsprecher (3 W, 4–8 Ω)
```

Der MAX98357A enthält DAC und Class-D-Verstärker in einem und treibt den Lautsprecher direkt an – kein separates DAC-Modul, kein AUX-Kabel, kein Bluetooth-Pairing nötig.

**Vorteile gegenüber Soundcore-Lösung:**
- Deutlich weniger Bauteile und Verkabelung.
- Kein Akku-/Ladethema mehr (das größte Sicherheitsrisiko im ganzen Projekt entfällt komplett) — siehe Punkt 15–18, die dadurch obsolet sind.
- Günstiger (~8–10 € für Verstärker + Lautsprecher zusammen).
- Der Soundcore 2 bleibt frei für seinen ursprünglichen Zweck (Bluetooth-Musik).

**Trade-off:** Weniger Lautstärke/Bass als der Soundcore 2 — für eine gesprochene Rufstimme in einem Zimmer aber ausreichend.

Stromversorgung: ESP32 und Verstärker+Lautsprecher laufen gemeinsam an einem normalen 5V-USB-Netzteil (kein eigener Akku nötig, siehe Punkt 19).

## 14. Audio-Verstärker (MAX98357A)

Der ESP32 sendet die Audiodaten per I2S-Bus an den MAX98357A. Dieser wandelt sie in ein analoges Signal um und verstärkt es direkt für den angeschlossenen Lautsprecher (Class-D-Verstärker, effizient, wenig Wärmeentwicklung). Kein zusätzliches DAC-Modul und kein externer Verstärker nötig.

## 15–18. Verworfene Idee: Soundcore-2-Integration und Akku-Automatik

> **Hinweis:** Diese vier Abschnitte stammen aus der ursprünglichen Planung, als der Soundcore 2 noch als Hauptlautsprecher vorgesehen war (siehe Entscheidung in Punkt 13). Wir verfolgen das nicht mehr weiter, da der Mini-Lautsprecher-Ansatz einfacher, günstiger und ohne Akku-Sicherheitsrisiko auskommt. Bleibt als Referenz erhalten, falls das Projekt später doch auf einen Bluetooth-Lautsprecher umgestellt werden soll.

## 15. Soundcore-Akku

Der Soundcore 2 besitzt einen eigenen Akku. Es wurde überlegt, den Soundcore 2 zu öffnen und seine interne Elektronik genauer zu untersuchen.

Mögliche Ziele wären:
- Akkuspannung untersuchen
- Ladeelektronik untersuchen
- eventuell Ladezustand/Akkuprozent auslesen
- prüfen, ob USB-A Strom ausgeben kann
- prüfen, wie der Lautsprecher beim Laden und gleichzeitigen Betrieb reagiert
- prüfen, ob ein automatischer Standby ein Problem darstellt

**Wichtig:** Der originale Akku und seine Lade-/Schutzschaltung sollen möglichst nicht einfach umgangen werden. Es soll keine eigene Lithium-Akku-Ladeschaltung direkt parallel an den internen Akku angeschlossen werden, ohne genau zu wissen, wie die vorhandene Elektronik funktioniert. Wenn der Lautsprecher geöffnet wird, soll zuerst nur gemessen und analysiert werden. Fotos der Platinen, Stecker, Akkuanschlüsse und Beschriftungen können später zur Analyse verwendet werden.

> **Sicherheitsrisiko (aus der Besprechung):** Dies ist der riskanteste Teil des Projekts. Eigene Ladeelektronik neben einer bestehenden Lithium-Schutzschaltung ist ein Brand-/Sicherheitsrisiko, wenn die Platine nicht zu 100 % verstanden wird. Empfehlung: zeitlich ganz ans Ende schieben (siehe Phase 12) und kein Blocker fürs Kernprodukt.

## 16. USB-Anschlüsse des Soundcore 2

Der Soundcore 2 besitzt USB-Anschlüsse. Es soll zunächst geprüft werden, welche Funktion der jeweilige USB-Anschluss tatsächlich hat. Insbesondere soll geprüft werden, ob der USB-A-Anschluss tatsächlich als 5-V-Stromausgang/Powerbank-Ausgang verwendet werden kann. Nicht davon ausgehen, dass ein USB-A-Anschluss automatisch Strom ausgibt — das soll mit Dokumentation bzw. Messungen überprüft werden.

## 17. Akku-Idee

Eine weitere Idee war, den ESP32 eventuell über den Akku des Soundcore 2 zu versorgen. Das wäre theoretisch möglich, wenn:

- eine geeignete Ausgangsspannung vorhanden ist
- die Stromversorgung ausreichend ist
- eine passende Spannungsregelung verwendet wird
- die Ladeelektronik berücksichtigt wird

Alternativ kann der ESP32 einen eigenen kleinen Akku erhalten. Eine besonders interessante spätere Variante wäre eine gemeinsame Energieversorgung mit Power-Path-Management.

## 18. Laden und Akku-Schonung

Der Soundcore 2 soll möglichst nicht unnötig dauerhaft bei 100 % geladen gehalten werden.

Eine mögliche spätere Lösung:
1. Lautsprecher läuft aus seinem Akku.
2. Zu einem bestimmten Zeitpunkt wird automatisch geladen.
3. Nach ausreichender Ladezeit wird die Stromversorgung wieder abgeschaltet.

Eine feste Ladezeit wie beispielsweise fünf Stunden wurde als Idee genannt, soll aber nicht blind umgesetzt werden. Besser wäre eine Lösung, die den tatsächlichen Ladezustand berücksichtigt, sofern dieser zuverlässig ausgelesen werden kann. Falls der ESP32 den Ladezustand des Soundcore 2 nicht auslesen kann, wäre ein zeitgesteuerter Ladevorgang möglich.

## 19. Stromversorgung des ESP32

Der ESP32 selbst soll möglichst dauerhaft mit Strom versorgt werden, da er Uhrzeit verwalten, WLAN-Verbindung halten, Gebetszeiten verwalten, Webserver bereitstellen und Zeitpläne überwachen muss. Der ESP32 benötigt dabei relativ wenig Leistung.

### Entscheidung (Stand 28.09.2026, aktualisiert): 4×AA-NiMH-Akku mit selbstgebauter, ESP32-gesteuerter Ladeschaltung

Da das Gerät dauerhaft laufen muss, ist ein Akku **nicht als alleinige Stromquelle** sinnvoll (wäre zu schnell leer), sondern als **Backup bei Stromausfall**.

**Ursprünglich geplant war ein LiPo-Akku mit Lade-/Boost-Modul (TP4056).** Stattdessen nutzen wir **4× vorhandene AA-NiMH-Akkus** (1,2 V/Zelle, in Reihe 4,8 V nominal, bis ~5,6 V frisch geladen) – das liegt bereits im Spannungsbereich, den ESP32 (über VIN) und MAX98357A brauchen, **ein Boost-Konverter ist damit nicht mehr nötig**.

Für „intelligentes" Laden (voll laden, dann stoppen) gibt es bei NiMH – anders als bei LiPo (TP4056) – **keine einfachen fertigen Hobby-Module**. Deshalb bauen wir die Ladesteuerung **selbst, per Software im ESP32**:

```
5V (Netzteil) → LM317-Konstantstromquelle (~200 mA, C/10-sicher) → MOSFET (vom ESP32 geschaltet) → Akkupack (4×AA)
```

**Ablauf:**
1. ESP32 erkennt über einen Spannungsteiler (GPIO 35), ob das Netzteil angeschlossen ist.
2. Wenn ja und Akkuspannung unter Schwellwert (~5,6–5,7 V) → MOSFET (GPIO 32) einschalten → laden.
3. ESP32 misst laufend weiter (GPIO 34, Akkuspannung) und schaltet den MOSFET aus, sobald der Schwellwert erreicht ist.
4. Danach gelegentliche Nachlade-Kontrolle (Selbstentladung ausgleichen).
5. Ohne Netzteil übernimmt der Akku automatisch über eine Dioden-ODER-Schaltung (Schottky-Diode), kein Rückfluss in den Akku.
6. Zusätzlich ein mechanischer Kippschalter für komplettes Ein/Aus.
7. **Akku-leer-Anzeige:** LED an GPIO 27, schaltet bei kritischem Ladezustand. Ladezustand soll zusätzlich in der Weboberfläche angezeigt werden (siehe Punkt 20).

**Bauteile:** 4×AA-NiMH-Akkus (vorhanden) + Batteriehalter, LM317 + ~6,8Ω/1W-Widerstand (Konstantstromquelle), N-Kanal-Logic-Level-MOSFET (z.B. IRLZ44N), Schottky-Diode, Kippschalter, 1 LED + 220Ω-Widerstand, 2× 100kΩ-Widerstand (Spannungsteiler Akku) + 2× 100kΩ-Widerstand (Spannungsteiler Netzteil-Erkennung).

**GPIO-Zuordnung:** GPIO 34 = Akkuspannung messen, GPIO 35 = Netzteil-Erkennung, GPIO 32 = Lade-MOSFET, GPIO 27 = Akku-leer-LED. *(Kollidiert nicht mit I2S: 25/26/22, oder SPI-SD: 5/18/23/19.)*

**Hinweis:** Die Software-Vollladungserkennung ist eine einfache Spannungsschwelle, kein präzises -ΔV-Verfahren wie bei teuren Ladegeräten – für NiMH bei niedrigem Ladestrom (C/10) unkritisch und ausreichend sicher.

## 20. Weboberfläche

Die Weboberfläche soll möglichst übersichtlich und für die Einrichtung geeignet sein.

Mögliche Startseite:
- aktuelle Uhrzeit / Datum
- nächster Gebetsruf + Countdown
- heutige Gebetszeiten
- WLAN-Status / Internetstatus / Diyanet-Status
- letzte Aktualisierung
- Lautstärke
- eventuell Akkuinformationen

Weitere Seiten: Startseite, Standort, Gebetszeiten, Verzögerungen, Audio, WLAN, System, Status/Diagnose.

## 21. Beispiel für Standortseite

- Land: Deutschland
- Region: Baden-Württemberg
- Stadt: Stuttgart
- Diyanet-Orts-ID: automatisch
- Koordinaten: automatisch/manuell (optional)
- Zeitzone: Europe/Berlin

## 22. Beispiel für Gebetszeiten-Seite

Heute:
- Fajr: 05:xx
- Dhuhr: 13:xx
- Asr: 16:xx
- Maghrib: 19:xx
- Isha: 21:xx

Daneben jeweils: Korrektur (-5 bis +5 Minuten oder frei einstellbar) und tatsächliche Abspielzeit (Diyanet-Zeit + Korrektur).

## 23. Beispiel für Audio-Seite

Für jedes Gebet: [Audio-Datei auswählen]. Zusätzlich: [Wiedergabe testen], [Audio hochladen].

## 24. Lautstärke

Die Lautstärke soll über die Weboberfläche einstellbar sein. Optional kann es unterschiedliche Lautstärken pro Gebet geben.

Beispiel:
- Fajr: 50 %
- Dhuhr: 65 %
- Asr: 65 %
- Maghrib: 75 %
- Isha: 60 %

Es soll geprüft werden, ob die Lautstärkeregelung am Soundcore 2 selbst, im Audio-DAC/ESP32 oder über die Audiodatei erfolgt.

## 25. Automatischer Ezan

Das Gerät prüft ständig die aktuelle Uhrzeit. Wenn die berechnete Abspielzeit erreicht wird:

1. passende Audio-Datei auswählen
2. Audio abspielen
3. Ezan über AUX an Soundcore 2 senden
4. nach Ende Wiedergabe beenden
5. auf nächsten Gebetsruf warten

Es soll verhindert werden, dass derselbe Ezan nach einem Neustart versehentlich mehrfach abgespielt wird. Dafür sollte gespeichert werden, welcher Gebetsruf an welchem Datum bereits abgespielt wurde.

## 26. Manuelle Wiedergabe

Über die Weboberfläche soll jederzeit ein Test möglich sein:

```
[ Fajr testen ] [ Dhuhr testen ] [ Asr testen ] [ Maghrib testen ] [ Isha testen ]
[ Stop ]
```

Damit kann die Installation getestet werden, ohne auf die nächste Gebetszeit warten zu müssen.

## 27. Systemstatus

Eine Diagnose-Seite soll zeigen:

- WLAN: Verbunden
- Internet: Verbunden
- Uhrzeit: Synchronisiert
- Diyanet: Aktuell
- Standort: Stuttgart
- Gebetszeiten: Aktuell
- Audio: Bereit
- SD-Karte: OK
- Lautsprecher: Audioausgang aktiv
- Letzte Synchronisation: Datum/Uhrzeit

## 28. Internetausfall

Das System soll möglichst robust sein.

**Wenn Internet weg ist:** keine Panik — lokale Uhr läuft weiter, gespeicherte Gebetszeiten werden verwendet, Ezan funktioniert weiterhin, Weboberfläche funktioniert im lokalen WLAN weiterhin.

**Wenn Internet wiederkommt:** NTP synchronisieren, Diyanet-Daten aktualisieren, neue Daten lokal speichern, Status aktualisieren.

## 29. Firmware-Updates

Später könnte die Weboberfläche auch Firmware-Updates ermöglichen (System → Update → Firmware-Datei auswählen → ESP32 aktualisiert sich). Das ist aber zunächst optional.

## 30. Sicherheitsidee

Das Setup-WLAN soll nur für die Ersteinrichtung bzw. Wiederherstellung aktiv sein. Die normale Weboberfläche sollte nicht unnötig offen ins Internet gestellt werden. Es ist nicht vorgesehen, den ESP32 von außen aus dem Internet erreichbar zu machen.

## 31. Noch offene technische Punkte

- Exakte aktuelle Diyanet-API und deren Nutzung.
- Welche API-Endpunkte für Länder, Regionen und Städte benötigt werden.
- Welche Diyanet-ID für einen ausgewählten Ort gespeichert werden muss.
- Wie oft Gebetszeiten sinnvoll aktualisiert werden.
- Welcher NTP-Zeitdienst verwendet wird.
- Welcher ESP32 genau verwendet werden soll. *(→ Standard-ESP32-WROOM, siehe Phase 1)*
- Wie die MP3-Dateien abgespielt werden. *(→ ESP32-audioI2S-Bibliothek)*
- Ob MP3-Decodierung direkt auf dem ESP32 oder über zusätzliche Hardware erfolgen soll. *(→ direkt auf dem ESP32, Software-Decoding)*
- Welche microSD-Karte bzw. Speicherlösung verwendet wird. *(→ einfaches SPI-microSD-Modul reicht)*
- Wie die Lautstärke am zuverlässigsten geregelt wird — vermutlich per Software (Lautstärke-Skalierung vor dem I2S-Ausgang), zu verifizieren beim Testen.
- Ob der MAX98357A-Mute-Pin fest beschaltet werden muss oder per Default schon aktiv ist (beim ersten Test prüfen).

## 32. Geplanter Entwicklungsweg

Das Projekt soll nicht alles auf einmal gebaut werden. Empfohlene Reihenfolge:

| Phase | Inhalt |
|---|---|
| 1 | ESP32 + WLAN + einfache Weboberfläche |
| 2 | NTP-Zeit und Zeitzone |
| 3 | Diyanet-Standortauswahl |
| 4 | Diyanet-Gebetszeiten abrufen und lokal speichern |
| 5 | Zeitplan und automatische Auslösung |
| 6 | Audioausgabe über ESP32 + MAX98357A + Lautsprecher |
| 7 | Audio-Dateiverwaltung |
| 8 | Verzögerungen und Lautstärke |
| 9 | Fallback bei Internetausfall |
| 10 | Status-/Diagnoseseite |
| 11 | Gehäuse und endgültige Installation |

*(Phasen „Soundcore 2 integrieren" und „Soundcore-Akku/Ladeautomatik" entfallen durch die Entscheidung in Punkt 13.)*

## 33. Ziel

Am Ende soll ein eigenständiges Gerät entstehen, das nach der Einrichtung praktisch selbstständig arbeitet. Die Einrichtung erfolgt über Handy/Browser. Der Benutzer wählt beispielsweise: Deutschland → Baden-Württemberg → Stuttgart. Danach holt der ESP32 die entsprechenden Diyanet-Daten, synchronisiert seine Uhrzeit über das Internet und speichert die benötigten Daten lokal.

Zu jeder Gebetszeit wird automatisch die zugehörige Ezan-Aufnahme über den fest angeschlossenen Lautsprecher (MAX98357A) abgespielt. Eigene Verzögerungen, Lautstärke und Audio-Dateien können jederzeit über die Weboberfläche verändert werden. Das Gerät soll auch bei einem vorübergehenden Internetausfall weiter funktionieren.

Der Soundcore 2 bleibt vom Projekt unberührt und steht weiterhin für seinen ursprünglichen Zweck (Bluetooth-Musik) zur Verfügung.

---

## Status

- Projektbeschreibung/Planung abgeschlossen.
- Diyanet-Datenquelle geklärt: `ezanvakti.emushaf.net` (frei, ohne Registrierung) statt offizieller API — **bestätigt durch offizielles Antragsformular + technisches Handbuch von Diyanet** (Kimlik-Nummer + Unterschrift nötig, Rate-Limit nur 5 Requests/Endpoint/Tag) — siehe Abschnitt 6.
- Audio-Ausgabe geklärt: eigener Mini-Lautsprecher über MAX98357A-I2S-Verstärker statt Soundcore 2 + AUX — siehe Abschnitt 13.
- Stromversorgung geklärt: Netzteil im Normalbetrieb + vorhandene 4×AA-NiMH-Akkus als Backup, mit selbstgebauter ESP32-gesteuerter Ladeschaltung (LM317 + MOSFET) und Akku-leer-LED — siehe Abschnitt 19.
- Verkabelung: Terminal-Adapter-Board (Schraubklemmen statt Löten/Jumperkabel) geplant — Pin-Anzahl (30 vs. 38 Pin) muss anhand eines Fotos des eigenen ESP32-Boards noch bestätigt werden.
- Gehäuse: wird vom Nutzer selbst konstruiert und 3D-gedruckt (siehe Phase 11) — kein Teil der Firmware-Planung.
- Phase 1 (ESP32 + WLAN-Ersteinrichtung + einfache Weboberfläche) wird im Unterordner `firmware/` umgesetzt.

**Einkaufsliste (aktuell):**
- ESP32-Board (vorhanden)
- ESP32-Terminal-Adapter-Board mit Schraubklemmen — **erst Pin-Anzahl (30/38) am eigenen Board prüfen, dann passenden kaufen** (~7 €)
- MAX98357A I2S-Verstärkermodul (~5 €)
- Kleiner Lautsprecher, 3 W, 4–8 Ω (~3–5 €)
- microSD-Kartenmodul (SPI) + microSD-Karte (~8–10 €)
- 4×AA-Batteriehalter (Akkus selbst vorhanden) (~2 €)
- LM317-Spannungsregler + 6,8Ω/1W-Widerstand (Ladestrom-Konstantquelle) (~2 €)
- N-Kanal-Logic-Level-MOSFET, z.B. IRLZ44N (Lade-Schalter) (~1–2 €)
- Schottky-Diode (Netz/Akku-Umschaltung) (~1 €)
- Kippschalter (Haupt-Ein/Aus) (~2 €)
- 1× LED (rot) + 220Ω-Widerstand (Akku-leer-Anzeige)
- 4× 100kΩ-Widerstand (2× Spannungsteiler Akku, 2× Spannungsteiler Netzteil-Erkennung)
- 5V-USB-Netzteil für den Normalbetrieb (~5 €)
- Jumperkabel (Female-Female) + Breadboard zum Prototypen (vor dem Festverkabeln auf dem Terminal-Adapter)

**Nächste Schritte für dich:**
1. Teile aus der Liste oben bestellen.
2. PlatformIO installieren (VS-Code-Extension oder CLI) zum Flashen.
3. Firmware aus `firmware/` auf den ESP32 flashen und Setup-WLAN testen.
