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
  Serial.println(getSchedule());
  delay(6000);
}

String getSchedule() {
  client.get("/");

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return "";
  }

  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.setCursor(0, 0);


  for (int i = 0; i < 1; i ++) {
    JsonObject item = doc.as<JsonArray>()[i];
    String routeId = item["routeId"];
    String direction = item["direction"];
    int minutesUntil = item["minutesUntil"];
    matrix.fillCircle(7, 7, 5, getLineColor(routeId));
    matrix.setCursor(5, 4);
    matrix.setTextColor(black);
    matrix.print(routeId);
    matrix.setCursor(15, 4);
    matrix.setTextColor(white);
    matrix.print(direction);
    matrix.print("  ");
    matrix.print(minutesUntil);
    matrix.println("min");
  }
  matrix.show();


  // for (JsonObject item : doc.as<JsonArray>()) {
  //   matrix.print("(");
  //   matrix.print((const char*)item["routeId"]);
  //   matrix.print(") ");
  //   matrix.print((const char*)item["direction"]);
  //   matrix.print(" ");
  //   matrix.print((int)item["minutesUntil"]);
  //   matrix.println("min");
  //   matrix.show();
  // }

  return response;
}

