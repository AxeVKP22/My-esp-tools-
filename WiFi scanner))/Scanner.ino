#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
extern "C" {
  #include "user_interface.h"
}


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// SDA = GPIO0, SCL = GPIO2
#define SDA_PIN 0
#define SCL_PIN 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("Scanning WiFi APs..."));
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
}

void loop() {
  int n = WiFi.scanNetworks();

  display.clearDisplay();
  display.setCursor(0, 0);

  if (n == 0) {
    display.println(F("Cant find APs"));
  } else {
    display.println("Find APs: " + String(n));
    for (int i = 0; i < n && i < 6; ++i) {
      display.setCursor(0, (i+1) * 10);
      display.println(WiFi.SSID(i));
    }
  }
  display.display();
  delay(5000);
}
