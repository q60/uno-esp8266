#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESPConfig.h>

const char* SERVER_PATH = "http://api.openweathermap.org/data/2.5/weather?lat=%g&lon=%g&units=metric&appid=%s";
const char* WIND_DIRECTIONS[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW", "N"};

WiFiClient client;

const char* windDegToDir(int deg) {
  return WIND_DIRECTIONS[(uint8_t) (round(8. / 359. * deg))];
}

void setup() {
  Serial.begin(9600);
  delay(3000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.pass);

  Serial.write(0x01);
  Serial.flush();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.write(0x02);
    Serial.flush();
  }

  Serial.write(0x03);
  Serial.print(WiFi.SSID());
  Serial.flush();
  delay(1000);
}

void loop() {
  HTTPClient http;

  char serverPath[128];
  snprintf(serverPath, sizeof(serverPath), SERVER_PATH, config.lat, config.lon, config.token);

  http.useHTTP10(true);
  http.begin(client, serverPath);

  int code = http.GET();

  if (code == 200) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, http.getStream());

    if (error) return;

    JsonObject weather = doc["weather"][0];
    const char* desc = weather["main"];

    JsonObject main = doc["main"];
    float temp = main["temp"];
    float feelsLike = main["feels_like"];
    int pressure = main["pressure"];
    int humidity = main["humidity"];
    int visibility = doc["visibility"];
    char visibilityPrettified[8];

    JsonObject wind = doc["wind"];
    float windSpeed = wind["speed"];
    int windDeg = wind["deg"];
    const char* windDir;

    const char* icon = weather["icon"];

    windDir = windDegToDir(windDeg);

    if (visibility == 10000) {
      strcpy(visibilityPrettified, "100%");
    } else {
      snprintf(visibilityPrettified, sizeof(visibilityPrettified), "%i m", visibility);
    }

    Serial.write(0x04);
    Serial.printf(
      "%s,%g,%g,%s,%g,%i,%g,%s,%s",
      desc,
      temp,
      feelsLike,
      windDir,
      windSpeed,
      humidity,
      pressure*0.75,
      visibilityPrettified,
      icon
    );

    Serial.flush();
  }

  http.end();

  delay(1000 * 60 * 30);
}
