#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 0
#define SCL_PIN 2

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

uint8_t beacon[] = {
  0x80, 0x00,
  0x00, 0x00,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01,
  0xDE, 0xAD, 0xBE, 0xEF, 0x00, 0x01,
  0x00, 0x00,

  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x64, 0x00,
  0x31, 0x04,

  0x00, 0x08, 'A','x','e','V','K','P','2','2',

  0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,

  0x03, 0x01, 0x01,

  0x05, 0x04, 0x00, 0x01, 0x00, 0x00
};



void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Beacon spam init"));
  display.display();

  WiFi.mode(WIFI_AP);
  wifi_promiscuous_enable(0);
  wifi_set_channel(1);

  delay(1000);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Beaconing..."));
  display.display();
}

void loop() {
  for (int i = 0; i < 50; i++) {
    wifi_send_pkt_freedom(beacon, sizeof(beacon), 0);
    delay(10);
  }
  delay(500);
}


