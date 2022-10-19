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
  attachInterrupt(digitalPinToInterrupt(downButton), downButtonListener, FALLING);
}

void loop() {
  setupWiFi();
  populate();
}

void populate() {
  StaticJsonDocument<2048> newDoc;
  doc = newDoc;

  if (rotating && on) {
    for (int i = 1; i < numArrivalsToShow && rotating && on; i ++) {
      // check again in case mode was changed during execution
      Serial.println("Getting schedule...");
      updateData();
      parseSettings(doc.as<JsonArray>()[0]);
      drawArrivals(0, i);
      delay(rotationTime * 900);
    }
  } else if (!rotating && on) {
    // check again in case mode was changed during execution
    Serial.println("Getting schedule...");
    updateData();
    parseSettings(doc.as<JsonArray>()[0]);
    drawArrivals(0, 1);
    delay(6000);
  } else if (!on) {
    matrix.fillScreen(black);
    matrix.show();
    Serial.println("Getting schedule...");
    updateData();
    parseSettings(doc.as<JsonArray>()[0]);
    delay(5000);
  }
}

void updateData() {
  String url = "/sign/";
  url += SIGN_ID;
  HttpClient client = HttpClient(wifiClient, serverAddress, port);
  int requestError = client.get(url);

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.println(response);

  if (requestError == 0) {
    DeserializationError jsonError = deserializeJson(doc, response);

    if (jsonError) {
      doc.clear();
      printMessage("Unreadable data");
      Serial.println(jsonError.c_str());
      setupWiFi();
      delay(500);
      updateData();
    }
  } else {
    Serial.print("Error requesting new data: ");
    Serial.println(requestError);
    printMessage("Request error");
    setupWiFi();
    delay(500);
    updateData();
  }

  client.stop();
}

void parseSettings (JsonObject settingsObject) {
  rotating = settingsObject["rotating"];
  on = (boolean)settingsObject["signOn"];
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

    // set up layout based on whether the sign is rotating. And if so, print the indices
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
    
    // draw the route icon for each arrival
    if (routeId[1] == 'X') {
      matrix.fillTriangle(
        xOrigin + 6, yOrigin + 2,
        xOrigin, yOrigin + 8,
        xOrigin + 12, yOrigin + 8,
        getLineColor(routeId.substring(0, 1)));
      matrix.fillTriangle(
        xOrigin, yOrigin + 8,
        xOrigin + 12, yOrigin + 8,
        xOrigin + 6, yOrigin + 14,
        getLineColor(routeId.substring(0, 1)));
    } else {
      matrix.fillCircle(xOrigin + 6, 8 + yOrigin, 5, getLineColor(routeId));
    }
    matrix.setCursor(xOrigin + 4, 5 + yOrigin);
    matrix.setTextColor(black);
    if (routeId == "GS") {
      matrix.print(routeId[1]);
    } else {
      matrix.print(routeId[0]);
    }

    // print headsign
    matrix.setCursor(xOrigin + 14, 5 + yOrigin);
    matrix.setTextColor(white);
    matrix.print(headsign);

    // print the arrival estimate
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
  int requestError = 1;
  String url = "/signpower/";
  url += SIGN_ID;
  HttpClient client = HttpClient(wifiClient, serverAddress, port);

  if (on) {
    url += "?power=false";
  } else {
    url += "?power=true";
  }
  requestError = client.put(url);

  if (requestError == 0) {
    requestError = client.responseStatusCode();
    if (requestError >= 200 || requestError < 300) {
      if (on) {
        on = false;
        printMessage("Sleep mode...");
        delay(1500);
        matrix.fillScreen(black);
        matrix.show();
      } else {
        on = true;
        populate();
      }
    } else {
      printMessage("Failed to save config");
    }
  } else {
    printMessage("Failed to save config");
  }
  client.stop();
}
