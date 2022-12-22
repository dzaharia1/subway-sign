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

void setup(void) {
  pinMode(LED_BUILTIN, OUTPUT);
  // Serial.begin(9600);
  // pinMode(upButton, INPUT_PULLUP);
  // pinMode(downButton, INPUT_PULLUP);
  setupMatrix();
  // setupWiFi();
  printMessage("Test/nTest");
}

void loop() {
}
