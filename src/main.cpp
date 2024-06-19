#include <RDA5807.h>
#include <WebServer.h>
#define I2C_SDA 21
#define I2C_SCL 22
#ifndef WIFI_SSID
  #define WIFI_SSID ""
#endif
#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD ""
#endif

RDA5807 rx;
WebServer server(80);

void setupWebServer();
void handleIndex();
void handleUpdate();
void handleNotFound();

void setupRDA5807();

void setup() {
  Serial.begin(115200);
  while (!Serial);
  setupRDA5807();

  Serial.println("\n*** Starting ***");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("connecting");
  }
  Serial.println("connected");
  Serial.println(WiFi.localIP());
  setupWebServer();
}

void loop() {
  server.handleClient();
}

void setupWebServer() {
  server.on("/", HTTP_GET, handleIndex);
  server.on("/", HTTP_POST, handleUpdate);
  server.onNotFound(handleNotFound);
  server.begin();
}

void handleIndex() {
  uint16_t freq = rx.getFrequency();
  int rssi = rx.getRssi();
  uint8_t vol = rx.getVolume();
  bool isStereo = rx.isStereo();
  String paramList = "\
<ul>\
  <li>Freq: <span id=\"freq\">" + String(float(freq) * 0.01) + "</span> MHz</li>\
  <li>RSSI: " + String(rssi) + " dbUv</li>\
  <li>Vol : <span id=\"vol\">" + String(vol) + "</span></li>\
  <li>Stereo: " + (isStereo ? "Yes" : "No") + "\
  </li>\
</ul>";
  
  String inputFreq = "\
<input \
  type=\"range\" \
  min=\"7610\" max=\"9490\" \
  value=\"" + String(freq) + "\" \
  name=\"freq\" \
  style=\"width: 300px\"\
>";
  String inputVol = "\
<input \
  type=\"range\" \
  min=\"0\" max=\"15\" \
  value=\"" + String(vol) + "\" \
  name=\"vol\" \
  style=\"width: 300px\"\
>";
  String formText ="\
<form method=\"POST\" action=\"/\">\
  <div>\
    <label for=\"freq\" style=\"width: 50px; display:inline-block;\">freq: </label>" + inputFreq + "\
  </div>\
  <div>\
    <label for=\"vol\" style=\"width: 50px; display:inline-block;\">vol: </label>" + inputVol + "\
  </div>\
  <input type=\"submit\" value=\"Submit\">\
</form>";
  String innerText = paramList + formText;
  String script = "<script>\
document.addEventListener('DOMContentLoaded', () => {\
  document.querySelector('input[name=freq]').addEventListener('change', (ev) => {\
    document.getElementById('freq').innerText = (ev.target.value * 0.01).toFixed(2);\
  });\
  document.querySelector('input[name=vol]').addEventListener('change', (ev) => {\
    document.getElementById('vol').innerText = ev.target.value;\
  });\
});\
</script>";
  String html = "\
<html>\
  <head>\
    <meta charset=\"utf-8\">\
    <title>ESP32 Radio</title>\
  </head>\
  <body>" +
    innerText + script + "\
  </body>\
</html>";
  server.send(200, "text/html", html);
}

void handleUpdate() {
  String freq = server.arg("freq");
  String vol = server.arg("vol");
  rx.setFrequency(freq.toInt());
  rx.setVolume(vol.toInt());
  server.sendHeader("Location", "/", true);
  server.send(303, "text/plain", "");
}

void handleNotFound() {
  Serial.println("HTTP " + server.uri());
  server.send(404, "text/plain", "404 Not Found");
}

void setupRDA5807() {
  Wire.begin(I2C_SDA, I2C_SCL);
  rx.setup();
  delay(500);
  rx.setVolume(1);
  rx.setMono(false);
  rx.setFrequency(8971);
}
