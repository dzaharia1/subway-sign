#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>
#include "configuration.h"
#include "colors.h"

char ssid[] = SSID;
char wifiPass[] = WIFI_PASS;
int keyIndex = 0;

int wifiStatus = WL_IDLE_STATUS;
char serverAddress[] = "192.168.1.81";
int port = 3333;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, port);
DynamicJsonDocument doc(4096);

// Adafruit_Protomatter matrix(
//   64, 4, 1, rgbPins, 4, addrPins, clockPin, latchPin, oePin, false);

void setup(void) {
  Serial.begin(9600);

  // Initialize matrix...
  ProtomatterStatus LEDStatus = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)LEDStatus);
  if(LEDStatus != PROTOMATTER_OK) {
    for(;;);
  }

  matrix.print("Start...");
  matrix.show();

  // set up wifi
  if (WiFi.status() == WL_NO_MODULE) {
   while (true);
  }

  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.begin(ssid, wifiPass);
    delay(1000);
  }
}

void loop() {
  Serial.println("Getting schedule...");
  getSchedule();
  delay(10000);
}

String getSchedule() {
  client.get("/");

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.println(response);

  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "";
  }

  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.setCursor(0, 0);

  // GFXcanvas16 canvases[3] = GFXcanvas16(matrix.width(), 11);

  for (int i = 0; i < 2; i ++) {
    JsonObject item = doc.as<JsonArray>()[i];
    String routeId = item["routeId"];
    String direction = item["direction"];
    int minutesUntil = item["minutesUntil"];
    int yOrigin = 16 * i;

    matrix.setTextColor(white);
    matrix.setCursor(1, 4 + yOrigin);
    matrix.print(i + 1);
    matrix.fillCircle(13, 7 + yOrigin, 5, getLineColor(routeId));
    matrix.setCursor(11, 4 + yOrigin);
    matrix.setTextColor(black);
    matrix.print(routeId);
    matrix.setCursor(21, 4 + yOrigin);
    matrix.setTextColor(white);
    matrix.print(direction);
    if (minutesUntil < 10) {
      matrix.print("  ");
    } else {
      matrix.print(" ");
    }
    if (minutesUntil == 0) {
      matrix.setTextColor(red123);
    }
    matrix.print(minutesUntil);
    matrix.println("min");
  }

  matrix.show();

  return response;
}

