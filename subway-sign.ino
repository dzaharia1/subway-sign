#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Adafruit_Protomatter.h>
#include "configuration.h"
#include "matrix.h"
#include "wifi.h"

// the document will contain the schedule. is updated by the updateData function
#define upButton 2
#define downButton 3
StaticJsonDocument<2048> doc;
String apiResponse;

boolean rotating = true;
boolean on = true;
int warnTime = 7;
int rotationTime = 5;
int numArrivalsToShow = 6;

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
  if (on) {
    Serial.println("Getting schedule...");
    updateData();
    parseSettings(doc.as<JsonArray>()[0]);
    populate();
  } else {
    matrix.fillScreen(black);
    delay(15000);
  }
}

void populate() {
    if (rotating && on) {
      for (int i = 1; i < numArrivalsToShow; i ++) {
        // check again in case mode was changed during execution
        if (rotating && on) {
          drawArrivals(0, i);
          delay(rotationTime * 1000);
        }
      }
    } else {
      drawArrivals(0, 1);
      delay(10000);
    }
}

void updateData() {
  String url = "/sign/";
  url += SIGN_ID;
  client.get(url);
  int statusCode = client.responseStatusCode();
  apiResponse = client.responseBody();
  Serial.println(apiResponse);

  doc.clear();
  DeserializationError error = deserializeJson(doc, apiResponse);

  if (statusCode != 200) {
    printMessage("Server unreachable");
    delay(1500);
    printMessage("Trying again");
    updateData();
  } else if (error) {
    printMessage("Parsing failed");
    delay(1000);
    printMessage(error.c_str());
    delay(5000);
    updateData();
  }
}

void parseSettings (JsonObject settingsObject) {
  rotating = settingsObject["rotating"];
  on = settingsObject["signOn"];
  warnTime = settingsObject["warnTime"];
  rotationTime = settingsObject["rotationTime"];
  numArrivalsToShow = settingsObject["numArrivals"];
}

// draw arrivals on the matrix
void drawArrivals(int firstIndex, int secondIndex) {
  JsonObject arrivalsToDraw[] = { doc.as<JsonArray>()[firstIndex + 1], doc.as<JsonArray>()[secondIndex + 1] };
  matrix.fillScreen(matrix.color565(0, 0, 0));
  matrix.setCursor(0, 0);

  for (int i = 0; i < 2; i ++) {
    JsonObject item = arrivalsToDraw[i];
    int yOrigin = 15 * i;
    int xOrigin;
    String routeId = item["routeId"];
    String headsign = item["headsign"];
    int minutesUntil = item["minutesUntil"];


    matrix.setTextColor(white);
    matrix.setTextSize(0);
    matrix.setTextWrap(false);

    if (rotating) {
      xOrigin = 6;
      headsign = headsign.substring(0, 13);
      matrix.setCursor(0, 5 + yOrigin);
      if (i == 0) {
        matrix.print(firstIndex + 1);
      } else {
        matrix.print(secondIndex + 1);
      }
    } else {
      headsign = headsign.substring(0, 14);
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
    if (routeId == "GS") {
      matrix.print(routeId[1]);
    } else {
      matrix.print(routeId[0]);
    }
    matrix.setCursor(xOrigin + 12, 5 + yOrigin);
    matrix.setTextColor(white);
    matrix.print(headsign);

    if (minutesUntil < 10) {
      matrix.setCursor(matrix.width() - 23, 5 + yOrigin);
    } else {
      matrix.setCursor(matrix.width() - 29, 5 + yOrigin);
    }

    if (minutesUntil <= warnTime) {
      matrix.setTextColor(getLineColor("B"), black);
    } else {
      matrix.setTextColor(white, black);
    }
    matrix.print(minutesUntil);
    matrix.print("min");
  }

  matrix.show();
}

void downButtonListener () {
  if (on) {
    on = false;
    printMessage("Sleep mode...");
    delay(3000);
    matrix.fillScreen(black);
    matrix.show();
  } else {
    on = true;
    populate();
  }
}

void upButtonListener () {
  rotating = !rotating;
  populate();
}
