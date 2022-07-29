#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "configuration.h"

char ssid[] = SSID;
char wifiPass[] = WIFI_PASS;

int wifiStatus = WL_IDLE_STATUS;
char serverAddress[] = "192.168.1.81";
int port = 3333;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, port);

void setupWiFi() {
  // set up wifi
  if (WiFi.status() == WL_NO_MODULE) {
   while (true);
  }

  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.begin(ssid, wifiPass);
    delay(1000);
  }
}