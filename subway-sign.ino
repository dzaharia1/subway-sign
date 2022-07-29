#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>
#include "configuration.h"
#include "matrix.h"

char ssid[] = SSID;
char wifiPass[] = WIFI_PASS;
int keyIndex = 0;

int wifiStatus = WL_IDLE_STATUS;
char serverAddress[] = "192.168.1.81";
int port = 3333;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, port);

// the document will contain the schedule. is updated by the getSchedule function
DynamicJsonDocument doc(4096);

// JsonObject schedule[5];

void setup(void) {
  Serial.begin(9600);

  Serial.print("Protomatter begin() status: ");
  Serial.println((int)LEDStatus);
  if(LEDStatus != PROTOMATTER_OK) {
    for(;;);
  }
  matrix.setRotation(2);

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
  for (int i = 1; i < 6; i ++) {
    drawArrivals(0, i);
    delay(5000);
  }
}

void getSchedule() {
  client.get("/");
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.println(response);

  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  } else {
    Serial.println("worked?");
  }
}

// draw arrivals on the matrix
void drawArrivals(int firstIndex, int secondIndex) {
  JsonObject arrivalsToDraw[] = { doc.as<JsonArray>()[firstIndex], doc.as<JsonArray>()[secondIndex] };
  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.setCursor(0, 0);

  for (int i = 0; i < 2; i ++) {
    JsonObject item = arrivalsToDraw[i];
    String routeId = item["routeId"];
    String direction = item["direction"];
    int minutesUntil = item["minutesUntil"];
    int yOrigin = 16 * i;

    matrix.setTextColor(white);
    matrix.setCursor(1, 4 + yOrigin);
    if (i == 0) {
      matrix.print(i + 1);
    } else {
      matrix.print(secondIndex + 1);
    }
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
      matrix.setTextColor(orangeBDFM);
    }
    matrix.print(minutesUntil);
    matrix.println("min");
  }

  matrix.show();
}

