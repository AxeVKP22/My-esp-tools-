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
#define BUTTON_PIN 1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte state = 2; //1 = Wifi scanner, 2 = menu, 3 = beacon
bool arrowState = true; //true = scanner, false = beacon spam
long mils = 0;
long lastMils = 0;

const unsigned char arrow [] PROGMEM = {
	0x20, 0x60, 0xe0, 0x60, 0x20
};

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

  0x00, 0x00,

  0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c,

  0x03, 0x01, 0x01,

  0x05, 0x04, 0x00, 0x01, 0x00, 0x00
};

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  WiFi.mode(WIFI_STA);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_set_channel(1);

  mils = millis();
}

void loop() {
  mils = millis();
  bool buttonState = digitalRead(BUTTON_PIN);

  if (state == 2) {

    if (arrowState and buttonState == LOW) {
      arrowState = false;
      lastMils = millis();
    }
    else if (!arrowState and buttonState == LOW) {
      arrowState = true;
      lastMils = millis();
    }

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 22);
    display.print(F("WiFi scanner"));

    display.setCursor(0, 42);
    display.print(F("Beacon spam"));

    if (arrowState) {
      display.drawBitmap(80, 23, arrow, 3, 5, SSD1306_WHITE);
      if ((mils - lastMils) >= 5000) {
        state = 1;
      }
    }
    else {
      display.drawBitmap(70, 43, arrow, 3, 5, SSD1306_WHITE);
      if ((mils - lastMils) >= 5000) {
        state = 3;
      }
      display.display();
    }
  }

  if (state == 1) {
      int n = WiFi.scanNetworks();

      display.clearDisplay();
      display.setCursor(0, 0);

      if (n == 0) {
        display.println(F("Cant find APs"));
      } 
      else {
        display.println("Find APs: " + String(n));
        for (int i = 0; i < n && i < 6; ++i) {
          display.setCursor(0, (i+1) * 10);
          display.println(WiFi.SSID(i));
        }
      }
      display.display();
      delay(5000);
    }

  if (state == 3) {
      display.clearDisplay();

      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);

      display.setCursor(0, 22);
      display.print(F("Beaconing..."));
      display.display();

      for (int i = 0; i < 100; i++) {
      String ssid = "hehehehhe" + String(i);
      int ssid_len = ssid.length();

      uint8_t packet[128];
      memcpy(packet, beacon, sizeof(beacon));

      packet[36] = 0x00;
      packet[37] = ssid_len;

      memcpy(&packet[38], ssid.c_str(), ssid_len);

      
      int packet_size = sizeof(beacon) + ssid_len;

      wifi_send_pkt_freedom(packet, packet_size, 0);
      delay(5);
    }

  delay(1000);
  }

}

