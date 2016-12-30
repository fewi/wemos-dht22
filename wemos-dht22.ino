#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <DHT.h>

#define DHTTYPE DHT22
#define DHTPIN 4

DHT dht(DHTPIN, DHTTYPE);

const int UPDATE_INTERVAL_MINUTES = 60;
const int WIFI_TIMEOUT = 60;

const char* host = "api.thingspeak.com";
const char* THINGSPEAK_API_KEY = "";

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(WIFI_TIMEOUT);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("Wemos-DHT22")) {
    Serial.println("failed to connect and hit timeout");
    setDeepSleep();
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected to access point");
  dht.begin();
}

void setDeepSleep() {
  Serial.println("putting ESP in deep sleep...");
  ESP.deepSleep(UPDATE_INTERVAL_MINUTES * 60 * 1000 * 1000 - millis(), WAKE_RF_DEFAULT);
  delay(3000);
}

void loop() {
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  delay(2000);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Prüfen ob eine gültige Zahl zurückgegeben wird. Wenn NaN (not a number) zurückgegeben wird, dann Fehler ausgeben.
  if (isnan(temperature) || isnan(humidity))
  {
    Serial.println("DHT22 konnte nicht ausgelesen werden");
  }
  else
  {
    Serial.print("Luftfeuchte: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperatur: ");
    Serial.print(temperature);
    Serial.println(" C");
  }

  // We now create a URI for the request
  String url = "/update?api_key=";
  url += THINGSPEAK_API_KEY;
  url += "&field1=";
  url += String(temperature);
  url += "&field2=";
  url += String(humidity);

  Serial.println("Sending values to thingspeak...");

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
  while (!client.available()) {
    delay(100);
  }

  setDeepSleep();

}


