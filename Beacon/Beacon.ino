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
#define BUTTON_PIN1 1
#define BUTTON_PIN2 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte state = 2;
byte menuIndex = 0;
long mils = 0;
long lastMils = 0;

const unsigned char arrow [] PROGMEM = {
  0x20, 0x60, 0xe0, 0x60, 0x20
};

uint8_t beacon[] = {
  0x80, 0x00, 0x00, 0x00,
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

// Буфер для сниффера
volatile bool newPacket = false;
char lastPacketInfo[64] = {0};

int currentChannel = 1;
long lastChannelSwitch = 0;

void ICACHE_FLASH_ATTR sniffer_callback(uint8_t *buf, uint16_t len) {
  if (len == 0 || buf == nullptr) return;

  uint8_t* sender_mac = buf + 10;
  sprintf(lastPacketInfo, "MAC: %02X:%02X:%02X:%02X:%02X:%02X Len: %d",
          sender_mac[0], sender_mac[1], sender_mac[2],
          sender_mac[3], sender_mac[4], sender_mac[5], len);
  newPacket = true;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  wifi_set_opmode(STATION_MODE);
  wifi_set_channel(currentChannel);
  wifi_set_promiscuous_rx_cb(sniffer_callback);
  mils = millis();
  state = 2;
}

void loop() {
  mils = millis();
  bool buttonState1 = digitalRead(BUTTON_PIN1) == LOW;
  bool buttonState2 = digitalRead(BUTTON_PIN2) == LOW;

  if (state == 2) {
    if (buttonState1 && (mils - lastMils) > 250) {
      menuIndex = (menuIndex + 1) % 4;
      lastMils = mils;
    }

    if (buttonState2 && (mils - lastMils) > 250) {
      state = menuIndex + 1;
      lastMils = mils;
      display.clearDisplay();
      display.display();
      delay(300);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.print(F("WiFi scanner"));
    display.setCursor(0, 28);
    display.print(F("sniffer"));
    display.setCursor(0, 46);
    display.print(F("Beacon spam"));

    if (menuIndex == 0) display.drawBitmap(100, 11, arrow, 3, 5, SSD1306_WHITE);
    if (menuIndex == 1) display.drawBitmap(100, 29, arrow, 3, 5, SSD1306_WHITE);
    if (menuIndex == 2) display.drawBitmap(100, 47, arrow, 3, 5, SSD1306_WHITE);

    display.display();
  }

  if (state == 1) {
    wifi_promiscuous_enable(0);
    int n = WiFi.scanNetworks();
    display.clearDisplay();
    display.setCursor(0, 0);
    if (n == 0) {
      display.println(F("Cant find APs"));
    } else {
      display.println("Find APs: " + String(n));
      for (int i = 0; i < n && i < 5; ++i) {
        display.setCursor(0, (i + 1) * 10);
        display.println(WiFi.SSID(i));
      }
    }
    display.display();
    delay(5000);
    state = 2;
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
    state = 2;
  }

  if (state == 4) {
    static bool sniffingStarted = false;
    if (!sniffingStarted) {
      wifi_promiscuous_enable(1);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Sniffing..."));
      display.display();
      sniffingStarted = true;
    }

    if (newPacket) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Sniffing...");
      display.setCursor(0, 20);
      display.println(lastPacketInfo);
      display.display();
      newPacket = false;
    }

    if (mils - lastChannelSwitch > 2000) {
      currentChannel++;
      if (currentChannel > 13) currentChannel = 1;
      wifi_set_channel(currentChannel);
      lastChannelSwitch = mils;
    }
  }
}
