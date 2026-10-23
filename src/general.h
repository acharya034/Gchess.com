#pragma once

#ifndef GENERAL_H
#define GENERAL_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <qrcode.h>
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
void drawToolbar(uint16_t &touchX, uint16_t &touchY, String text = "", uint16_t bg_color = TFT_BLACK, bool printBackground = true, bool backButton = true, bool settingButton = true, bool wifiIcon = true, bool batteryIcon = true);

// QR Code APIs
#define QR_MAX_BUFFER_SIZE 1024

struct QRCodeData {
    QRCode qr;
    uint8_t buffer[QR_MAX_BUFFER_SIZE];
    bool valid = false;
};

// Generate and store the QR matrix
bool generateQRCode(
    QRCodeData &code,
    const char *text,
    uint8_t version = 4,
    uint8_t ecc = 1
);

// Draw a previously generated QR code
bool printQRCode(
    QRCodeData &code,
    int16_t x,
    int16_t y,
    int16_t width,
    bool printLogo = false,
    uint16_t backgroundColor = TFT_WHITE,
    uint16_t codeColor = TFT_BLACK
);

#endif // GENERAL_H
