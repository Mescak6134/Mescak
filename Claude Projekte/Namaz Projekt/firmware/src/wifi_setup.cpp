#include "wifi_setup.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>

namespace WifiSetup {
namespace {

// Setup-WLAN: nur fuer die Ersteinrichtung aktiv (siehe Punkt 4 im Projekt-
// dokument). Passwort hier dokumentiert, da das Geraet kein eigenes Display
// hat, um es anzuzeigen.
const char* AP_SSID = "Gebetsruf-Setup";
const char* AP_PASSWORD = "namaz1234";
const char* HOSTNAME = "gebetsruf";
const byte DNS_PORT = 53;
const unsigned long STA_CONNECT_TIMEOUT_MS = 12000;

WebServer server(80);
DNSServer dnsServer;
Preferences prefs;
bool apMode = false;

String htmlPage(const String& title, const String& body) {
  String html = "<!DOCTYPE html><html lang=\"de\"><head><meta charset=\"utf-8\">";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<title>" + title + "</title><style>";
  html += "body{font-family:sans-serif;max-width:480px;margin:2rem auto;padding:0 1rem;"
          "background:#f5f5f0;color:#222}";
  html += "h1{font-size:1.3rem}";
  html += "label{display:block;margin-top:1rem;font-weight:bold}";
  html += "input,select{width:100%;padding:0.5rem;margin-top:0.25rem;box-sizing:border-box}";
  html += "button{margin-top:1.5rem;padding:0.7rem 1.2rem;background:#2b7a4b;color:#fff;"
          "border:none;border-radius:4px;font-size:1rem}";
  html += ".card{background:#fff;border-radius:8px;padding:1rem 1.2rem;"
          "box-shadow:0 1px 3px rgba(0,0,0,0.1);margin-top:1rem}";
  html += "</style></head><body><h1>Gebetsruf-Lautsprecher</h1>" + body + "</body></html>";
  return html;
}

void startAp() {
  apMode = true;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  delay(200);
  IPAddress apIp = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", apIp);
  Serial.printf("Setup-WLAN aktiv: SSID=%s PW=%s IP=%s\n", AP_SSID, AP_PASSWORD,
                apIp.toString().c_str());
}

bool tryConnectSta(const String& ssid, const String& password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < STA_CONNECT_TIMEOUT_MS) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

void handleRoot() {
  if (apMode) {
    String body =
        "<div class=\"card\"><p>Noch kein WLAN eingerichtet. Waehle dein Heim-WLAN "
        "und gib das Passwort ein.</p>"
        "<form action=\"/save\" method=\"POST\">"
        "<label>WLAN-Netzwerk</label>"
        "<select name=\"ssid\" id=\"ssid\"><option>Wird geladen...</option></select>"
        "<label>Passwort</label><input type=\"password\" name=\"password\">"
        "<button type=\"submit\">Speichern &amp; verbinden</button></form></div>"
        "<script>"
        "fetch('/scan').then(r=>r.json()).then(list=>{"
        "const sel=document.getElementById('ssid');sel.innerHTML='';"
        "list.forEach(n=>{const o=document.createElement('option');"
        "o.value=n;o.textContent=n;sel.appendChild(o);});"
        "});"
        "</script>";
    server.send(200, "text/html", htmlPage("WLAN-Einrichtung", body));
  } else {
    String body = "<div class=\"card\">";
    body += "<p><b>Status:</b> WLAN verbunden</p>";
    body += "<p><b>IP-Adresse:</b> " + WiFi.localIP().toString() + "</p>";
    body += "<p><b>Netzwerk:</b> " + WiFi.SSID() + "</p>";
    body += "<p><b>Laufzeit:</b> " + String(millis() / 1000) + " s</p>";
    body += "<p>Weitere Funktionen (Standort, Gebetszeiten, Audio) folgen in "
            "den naechsten Ausbaustufen.</p>";
    body += "</div>";
    server.send(200, "text/html", htmlPage("Status", body));
  }
}

void handleScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  for (int i = 0; i < n; i++) {
    if (i > 0) json += ",";
    json += "\"" + WiFi.SSID(i) + "\"";
  }
  json += "]";
  server.send(200, "application/json", json);
}

void handleSave() {
  if (!server.hasArg("ssid") || server.arg("ssid").length() == 0) {
    server.send(400, "text/plain", "ssid fehlt");
    return;
  }
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("password", password);
  prefs.end();

  String body = "<div class=\"card\"><p>Gespeichert. Das Geraet startet neu und "
                "verbindet sich mit <b>" + ssid + "</b>.</p>"
                "<p>Falls die Verbindung fehlschlaegt, oeffnet sich das "
                "Setup-WLAN automatisch erneut.</p></div>";
  server.send(200, "text/html", htmlPage("Gespeichert", body));
  delay(1500);
  ESP.restart();
}

void handleNotFound() {
  if (apMode) {
    // Captive-Portal-Verhalten: jede unbekannte Adresse auf die Setup-Seite lenken.
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  } else {
    server.send(404, "text/plain", "Nicht gefunden");
  }
}

} // namespace

void begin() {
  Serial.begin(115200);
  delay(200);

  prefs.begin("wifi", true);
  String savedSsid = prefs.getString("ssid", "");
  String savedPassword = prefs.getString("password", "");
  prefs.end();

  bool connected = false;
  if (savedSsid.length() > 0) {
    Serial.printf("Verbinde mit gespeichertem WLAN: %s\n", savedSsid.c_str());
    connected = tryConnectSta(savedSsid, savedPassword);
  }

  if (connected) {
    apMode = false;
    Serial.printf("Verbunden. IP: %s\n", WiFi.localIP().toString().c_str());
    if (MDNS.begin(HOSTNAME)) {
      Serial.printf("mDNS aktiv: http://%s.local\n", HOSTNAME);
    }
  } else {
    Serial.println("Kein WLAN verbunden, starte Setup-Modus.");
    startAp();
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/scan", HTTP_GET, handleScan);
  server.on("/save", HTTP_POST, handleSave);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  if (apMode) {
    dnsServer.processNextRequest();
  }
  server.handleClient();
}

bool isApMode() {
  return apMode;
}

} // namespace WifiSetup
