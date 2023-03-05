#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include <SD.h>

#define SD_CS 10

MCUFRIEND_kbv tft(A3, A2, A1, A0, A4);

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++){
    if (data.charAt(i) == separator || i == maxIndex){
        found++;
        strIndex[0] = strIndex[1] + 1;
        strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

uint16_t read16(File f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read();
  ((uint8_t *)&result)[1] = f.read();
  return result;
}

void setup() {
  Serial.begin(9600);
  delay(3000);


  if (!SD.begin(SD_CS)) {
    return;
  }

  unsigned id = tft.readID();
  tft.begin(id);

  tft.setRotation(1);
  tft.fillScreen(0x0000);
  tft.setCursor(0, 0);
  tft.setTextColor(0xF800);
  tft.setTextSize(1);
}

void drawIcon(const char* icon, int x, int y) {
  File file = SD.open(icon);

  for (int i = 0; i < 64*64; i++) {
    tft.drawPixel(x + i % 64, y + i / 64, read16(file));
  }

  file.close();
}

void loop() {
  String weather;

  if (Serial.available()) {
    switch (Serial.read()) {
      case 0x01:
        tft.fillScreen(0x0000);
        tft.setTextColor(0xF800);
        tft.println("connecting");
        break;

      case 0x02:
        tft.print(".");
        break;

      case 0x03:
        tft.println(".");
        tft.print("connected to: ");
        tft.println(Serial.readString());
        break;

      case 0x04:
        tft.fillScreen(0xFFFF);
        tft.setCursor(0, 0);
        tft.setTextSize(2);

        weather = Serial.readString();
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather, ',', 0));
        tft.setTextColor(0x03D4);
        tft.print("Temp: ");
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather, ',', 1) + " C (" + getValue(weather, ',', 2) + " C)");
        tft.setTextColor(0x03D4);
        tft.print("Wind: ");
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather,',', 3) + " " + getValue(weather, ',', 4) + " m/s");
        tft.setTextColor(0x03D4);
        tft.print("Humidity: ");
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather, ',', 5) + "%");
        tft.setTextColor(0x03D4);
        tft.print("Pressure: ");
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather, ',', 6) + " mmHg");
        tft.setTextColor(0x03D4);
        tft.print("Visibility: ");
        tft.setTextColor(0xFDE0);
        tft.println(getValue(weather, ',', 7));
        tft.setTextSize(1);
        tft.setTextColor(0xE1E6);
        tft.println("Your city");

        drawIcon(getValue(weather, ',', 8).c_str(), 160-32, 120);
        break;

      default:
        while (Serial.available()) Serial.read();
        break;
    }
  }
}
