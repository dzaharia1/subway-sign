#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include "configuration.h"
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883

char ssid[] = SSID;
char wifiPass[] = WIFI_PASS;

int wifiStatus = WL_IDLE_STATUS;
char serverAddress[] = "subway-arrivals.herokuapp.com";
int port = 80;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverAddress, port);
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Publish logFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/subway-sign-log");

void writeLog(String value) {
  char buffer[100];
    Serial.println(value);
  value.toCharArray(buffer, 100);
  if (!logFeed.publish(buffer)) {
    Serial.println("Failed to publish to Adafruit IO");
  } else {
  }
}

void setupWiFi() {
  // set up wifi
  if (WiFi.status() == WL_NO_MODULE) {
   while (true);
  }

  while (wifiStatus != WL_CONNECTED) {
    wifiStatus = WiFi.begin(ssid, wifiPass);
    delay(1000);
  }

  if (mqtt.connected()) {
    return;
  }

  int8_t ret;
  Serial.println("Connecting to MQTT...");
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retryingt MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
  }

  Serial.println("MQTT Connected!");
}