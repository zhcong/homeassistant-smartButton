
#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/adc.h"
#include <WiFi.h>
#include "homeassistant.h"

#define BUTTON_PIN 18

#define RST_PIN 34
#define LED_PIN 21

#define BUTTON_PIN_BITMASK 0x40000

#define SSID "you ssid"
#define PASSWD "you password"
Scheduler runner;

char mqttHost[] = "192.168.1.1";
int mqttPort = 1883;
String name = "智能按钮1";
String id = "smartSwitch1";

RTC_DATA_ATTR bool firstInit = true;

void pinInit() {
  pinMode(RST_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
}

bool wifiInit() {
  WiFi.disconnect(true, false);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.begin(SSID, PASSWD);
  int count = 500;
  while (count > 0) {
    count--;
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(10);
  }
  return false;
}

void gotoSleep() {
  delay(1000);
  WiFi.disconnect(true, false);
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_LOW);
  adc_power_release();
  analogWrite(LED_PIN, 0);
  esp_deep_sleep_start();
}

void homeassistantStart() {
  runner.init();
  homeassistantInit(id, name, mqttHost, mqttPort, &onSwitchChange, &runner, true, SWITCH_TYPE, "");
}

void onSwitchChange(bool n) {}

void setup() {
  // Serial.begin(115200);
  pinInit();
  if (!digitalRead(RST_PIN)) {
    // Serial.println("rest.");
    delay(1000);
    ESP.restart();
  }
  analogWrite(LED_PIN, 5);
  if (!wifiInit()) {
    // Serial.println("connect fail.");
    gotoSleep();
  }
  if (firstInit) {
    firstInit = false;
    homeassistantStart();
  } else {
    homeassistantStart();
    homeassistantSwitch(true);
    delay(100);
    homeassistantSwitch(false);
  }
  gotoSleep();
}

void loop() {
}
