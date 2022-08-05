#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>
// #include "configuration.h"
#include "matrix.h"
#include "wifi.h"

// the document will contain the schedule. is updated by the getSchedule function
StaticJsonDocument<1024> doc;
#define upButton 2
#define downButton 3

void setup(void) {
  Serial.begin(9600);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  setupMatrix();
  setupWiFi();
  attachInterrupt(digitalPinToInterrupt(upButton), upButtonListener, FALLING);
}

void loop() {
  Serial.println("Getting schedule...");
  getSchedule();
  for (int i = 1; i < 6; i ++) {
    drawArrivals(0, i);
    delay(6000);
  }
}

void upButtonListener () {
  Serial.println("teehee!");
  if (matrix.getRotation() == 0) {
    matrix.setRotation(2);
  } else {
    matrix.setRotation(0);
  }
  drawArrivals(0, 1);
  matrix.show();
}

void getSchedule() {
  client.get("/");
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.println(response);

  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    matrix.print("deserializeJson() failed: ");
    matrix.println(error.c_str());
    return;
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
    // if (direction == "Northbound") {
    //   direction = "North";
    // } else if (direction == "Southbound") {
    //   direction  = "South";
    // } else {
    //   direction = direction.substring(0, 4);
    // }
    int minutesUntil = item["minutesUntil"];
    int yOrigin = 15 * i;

    matrix.setTextColor(white);
    matrix.setTextSize(0);
    matrix.setTextWrap(false);
    matrix.setCursor(0, 5 + yOrigin);
    if (i == 0) {
      matrix.print(firstIndex + 1);
    } else {
      matrix.print(secondIndex + 1);
    }
    matrix.fillCircle(11, 8 + yOrigin, 5, getLineColor(routeId));
    matrix.setCursor(9, 5 + yOrigin);
    matrix.setTextColor(black);
    matrix.print(routeId);
    matrix.setCursor(18, 5 + yOrigin);
    matrix.setTextColor(white);
    matrix.print(direction);
    if (minutesUntil < 10) {
      matrix.setCursor(53, 5 + yOrigin);
    } else {
      matrix.setCursor(47, 5 + yOrigin);
    }
    if (minutesUntil == 0) {
      matrix.setTextColor(orangeBDFM);
    }
    matrix.print(minutesUntil);
    matrix.println("m");
  }

  matrix.show();
}
