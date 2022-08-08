#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>
#include "configuration.h"
#include "matrix.h"
#include "wifi.h"

// the document will contain the schedule. is updated by the getSchedule function
StaticJsonDocument<1024> doc;
#define upButton 2
#define downButton 3

boolean rotating = true;

void setup(void) {
  Serial.begin(9600);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  setupMatrix();
  setupWiFi();
  attachInterrupt(digitalPinToInterrupt(upButton), upButtonListener, FALLING);
  attachInterrupt(digitalPinToInterrupt(downButton), downButtonListener, FALLING);
}

void loop() {
  Serial.println("Getting schedule...");
  getSchedule();

  if (rotating) {
    for (int i = 1; i < 6; i ++) {
      drawArrivals(0, i);
      delay(5000);
    }
  } else {
    drawArrivals(0, 1);
    delay(15000);
  }
}

void getSchedule() {
  String url = "/sign/";
  url += SIGN_ID;
  client.get(url);
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.println(response);

  DeserializationError error = deserializeJson(doc, response);

  if (error) {
    matrix.print("deserializeJson() failed: ");
    matrix.println(error.c_str());
    delay(5000);
    getSchedule();
  }
}

// draw arrivals on the matrix
void drawArrivals(int firstIndex, int secondIndex) {
  JsonObject arrivalsToDraw[] = { doc.as<JsonArray>()[firstIndex], doc.as<JsonArray>()[secondIndex] };
  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.setCursor(0, 0);

  for (int i = 0; i < 2; i ++) {
    JsonObject item = arrivalsToDraw[i];
    int yOrigin = 15 * i;
    int xOrigin;
    String routeId = item["routeId"];
    String direction = item["headsign"];
    int minutesUntil = item["minutesUntil"];


    matrix.setTextColor(white);
    matrix.setTextSize(0);
    matrix.setTextWrap(false);

    if (rotating) {
      xOrigin = 6;
      direction = direction.substring(0, 12);
      matrix.setCursor(0, 5 + yOrigin);
      if (i == 0) {
        matrix.print(firstIndex + 1);
      } else {
        matrix.print(secondIndex + 1);
      }
    } else {
      direction = direction.substring(0, 14);
      xOrigin = 0;
    }
    
    if (routeId[1] == 'X') {
      matrix.fillTriangle(
        xOrigin + 5, yOrigin + 2,
        xOrigin - 1, yOrigin + 8,
        xOrigin + 11, yOrigin + 8,
        getLineColor(routeId.substring(0, 1)));
      matrix.fillTriangle(
        xOrigin - 1, yOrigin + 8,
        xOrigin + 11, yOrigin + 8,
        xOrigin + 5, yOrigin + 14,
        getLineColor(routeId.substring(0, 1)));
    } else {
      matrix.fillCircle(xOrigin + 5, 8 + yOrigin, 5, getLineColor(routeId));
    }
    matrix.setCursor(xOrigin + 3, 5 + yOrigin);
    matrix.setTextColor(black);
    matrix.print(routeId[0]);
    matrix.setCursor(xOrigin + 13, 5 + yOrigin);
    matrix.setTextColor(white);
    matrix.print(direction);

    if (minutesUntil < 10) {
      matrix.setCursor(matrix.width() - 24, 5 + yOrigin);
    } else {
      matrix.setCursor(matrix.width() - 30, 5 + yOrigin);
    }

    matrix.setTextColor(white, black);
    matrix.print(minutesUntil);
    matrix.println("min");
  }

  matrix.show();
}

void upButtonListener () {
  if (matrix.getRotation() == 0) {
    matrix.setRotation(2);
  } else {
    matrix.setRotation(0);
  }
  drawArrivals(0, 1);
  matrix.show();
}

void downButtonListener () {
  rotating = !rotating;
  drawArrivals(0, 1);
}
