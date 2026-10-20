#pragma once

#ifndef GENERAL_H
#define GENERAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <TFTInput.h>
#include <pages.h>

// Unified layout for your UI rendering loop
struct WifiUIData {
  String ssid;
  String passkey; // Left empty unless matching a known network
  int data;       // (Status * 10) + Strength
};

// Global variables (Shared via extern linkages)
extern int connectedWifiStrength; 
extern uint8_t pressedButton;
extern unsigned long buttonReleasedStartTime;
extern WiFiMulti wifiMulti;
extern Preferences preferences;
extern TFT_eSPI display;
extern TFT_eSprite sprite; 

// Core System Wi-Fi APIs
void initWifiManager();
void maintainWifiConnection();
bool addUserNetwork(const char* ssid, const char* password); // Tests connection before saving!
void clearAllStoredNetworks();
int populateWifiSettingsPage(WifiUIData* outUiList, int maxListSize);
bool connectToWifi(const char* username, const char* passkey);

// Core UI & Drawing Graphics APIs
void buttonPressed(uint8_t buttonId);
bool isButtonReleased(uint8_t buttonId, unsigned long BUTTON_RELEASED_TIMEOUT = 200);
bool touchContained(uint16_t touchX, uint16_t touchY, int32_t x, int32_t y, int32_t width, int32_t height);
uint8_t getBatteryLevel(); 

void drawWifiIndicator(uint8_t strength, int32_t x, int32_t y, uint8_t size, int32_t bg_color = TFT_BLACK);
void drawBatteryIndicator(uint8_t strength, int32_t x, int32_t y, uint8_t size, int32_t bg_color = TFT_BLACK);
void drawbackbutton(int32_t x, int32_t y, int32_t width, int32_t height, uint16_t &touchX, uint16_t &touchY, uint16_t color, uint16_t bg_color);
void drawToolbar(uint16_t &touchX, uint16_t &touchY, uint16_t bg_color = TFT_BLACK);

#endif // GENERAL_H
