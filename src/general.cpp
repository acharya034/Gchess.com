#include "general.h"

// Instantiate the global object handles here safely
int connectedWifiStrength = 0;
uint8_t pressedButton = 0;
unsigned long buttonReleasedStartTime = 0;

WiFiMulti wifiMulti;
Preferences preferences;
TFT_eSPI display = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&display);

const char* default_ssid = "Gchess";
const char* default_password = "Gchess@123";

// Local internal setup function to populate WiFiMulti on boot with infinite storage keys
void initWifiManager() {
  WiFi.mode(WIFI_STA);
  
  preferences.begin("wifi_config", true);
  int count = preferences.getInt("count", 0);
  
  for (int i = 0; i < count; i++) {
    String ssidKey = "s_" + String(i);
    String passKey = "p_" + String(i);
    
    String storedSSID = preferences.getString(ssidKey.c_str(), "");
    String storedPass = preferences.getString(passKey.c_str(), "");
    
    if (storedSSID.length() > 0) {
      wifiMulti.addAP(storedSSID.c_str(), storedPass.c_str());
    }
  }
  preferences.end();
}

// Single non-blocking engine running inside main loop() updating the global integer
void maintainWifiConnection() {
  static unsigned long lastCheckTime = 0;
  if (millis() - lastCheckTime < 3000) return; 
  lastCheckTime = millis();

  // Run the background connection handling engine
  wifiMulti.run();

  // Dynamically calculate the active link's strength for your global toolbar variable
  if (WiFi.status() == WL_CONNECTED) {
    long rssi = WiFi.RSSI();
    if (rssi >= -67)      connectedWifiStrength = 3;
    else if (rssi >= -80) connectedWifiStrength = 2;
    else                  connectedWifiStrength = 1;
  } else {
    connectedWifiStrength = 0; 
  }
}

// Verifies credentials by trying to connect first. Saves to flash ONLY if successful.
bool addUserNetwork(const char* ssid, const char* password) {
  // 1. Try connecting to the provided network credentials first
  bool success = connectToWifi(ssid, password);
  
  // 2. If connection fails, reject saving and return false immediately
  if (!success) {
    return false;
  }
  
  // 3. If connection succeeds, permanently store it into NVS Flash Storage
  preferences.begin("wifi_config", false);
  int totalSaved = preferences.getInt("count", 0);
  
  String ssidKey = "s_" + String(totalSaved);
  String passKey = "p_" + String(totalSaved);
  
  preferences.putString(ssidKey.c_str(), ssid);
  preferences.putString(passKey.c_str(), password);
  
  preferences.putInt("count", totalSaved + 1);
  preferences.end();
  
  // 4. Pass directly into the active auto-reconnect list
  wifiMulti.addAP(ssid, password);
  
  return true;
}

// Complete NVS Namespace Wipe out
void clearAllStoredNetworks() {
  preferences.begin("wifi_config", false);
  preferences.clear();
  preferences.end();
}

// Scanning and algorithmic classification function (No static limit bounds)
int populateWifiSettingsPage(WifiUIData* outUiList, int maxListSize) {
  preferences.begin("wifi_config", true);
  int totalSaved = preferences.getInt("count", 0);
  
  String* cachedSSIDs = new String[totalSaved];
  String* cachedPasses = new String[totalSaved];
  
  for (int i = 0; i < totalSaved; i++) {
    String ssidKey = "s_" + String(i);
    String passKey = "p_" + String(i);
    cachedSSIDs[i] = preferences.getString(ssidKey.c_str(), "");
    cachedPasses[i] = preferences.getString(passKey.c_str(), "");
  }
  preferences.end();

  int foundNetworks = WiFi.scanNetworks();
  int uiCount = 0;

  for (int i = 0; i < foundNetworks && uiCount < maxListSize; i++) {
    String currentSSID = WiFi.SSID(i);
    long rssi = WiFi.RSSI(i);
    
    if (currentSSID.length() == 0) continue; 

    outUiList[uiCount].ssid = currentSSID;
    outUiList[uiCount].passkey = ""; 

    int strength = 0;
    if (rssi >= -67)      strength = 3;
    else if (rssi >= -80) strength = 2;
    else                  strength = 1;

    int statusType = 0; // 0 = Unknown
    
    for (int j = 0; j < totalSaved; j++) {
      if (currentSSID == cachedSSIDs[j]) {
        statusType = 1; // 1 = Known
        outUiList[uiCount].passkey = cachedPasses[j];
        break;
      }
    }

    if (WiFi.status() == WL_CONNECTED && currentSSID == WiFi.SSID()) {
      statusType = 2; // 2 = Connected
    }

    outUiList[uiCount].data = (statusType * 10) + strength;
    uiCount++;
  }

  delete[] cachedSSIDs;
  delete[] cachedPasses;
  WiFi.scanDelete(); 
  return uiCount; 
}

// Blocking function to attempt network authentication connection
bool connectToWifi(const char* username, const char* passkey) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(username, passkey);
  
  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000; // 10-second authentication timeout window
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(200); 
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  } else {
    WiFi.disconnect(); 
    return false;
  }
}

void buttonPressed(uint8_t buttonId) {
    buttonReleasedStartTime = millis();
    pressedButton = buttonId;
}

bool isButtonReleased(uint8_t buttonId, unsigned long BUTTON_RELEASED_TIMEOUT) {
    if (pressedButton == buttonId && (millis() - buttonReleasedStartTime >= BUTTON_RELEASED_TIMEOUT)) {
        buttonReleasedStartTime = 0; 
        pressedButton = 0; 
        return true;
    }
    return false;
}

bool touchContained(uint16_t touchX, uint16_t touchY, int32_t x, int32_t y, int32_t width, int32_t height) {
    return (touchX > x && touchX <= x + width && touchY > y && touchY <= y + height);
}

uint8_t getBatteryLevel() {
    static uint8_t batteryLevel = 0;
    static unsigned long batteryCheckTimmer = 0;
    if (millis() - batteryCheckTimmer < 10000) return batteryLevel;
    // TODO:
    // find battery level here.
    return 3; // temp
}

void drawWifiIndicator(uint8_t strength, int32_t x, int32_t y, uint8_t size, int32_t bg_color) {
    uint16_t scaledSize = (uint16_t)size * 30 / 150;
    if (scaledSize == 0) scaledSize = 1;

    uint16_t active   = TFT_WHITE;
    uint16_t inactive = 0x8A795D;

    int cx = x + scaledSize / 2;
    int cy = y + scaledSize - scaledSize / 8;
    int r  = scaledSize / 2;

    sprite.fillSmoothCircle(cx, cy, max(1, (int)scaledSize / 16), (strength > 0) ? active : inactive, bg_color);

    for (uint8_t i = 0; i < 3; i++) {
        int radius = r * (i + 1) / 3;
        if (radius < 1) continue;
        uint16_t color = (i < strength) ? active : inactive;
        sprite.drawSmoothArc(cx, cy, radius, radius, 225, 315, color, bg_color);
    }
}

void drawBatteryIndicator(uint8_t strength, int32_t x, int32_t y, uint8_t size, int32_t bg_color) {
    uint16_t scaledWidth = (uint16_t)size * 30 / 150;
    if (scaledWidth < 6) scaledWidth = 6; 

    uint16_t scaledHeight = scaledWidth * 15 / 30;
    if (scaledHeight < 4) scaledHeight = 4;

    uint16_t bodyColor = TFT_BLUE;
    uint16_t activeColor = TFT_WHITE; 
    int8_t segmentsToFill = 0;

    if (strength == 0) {
        segmentsToFill = 1;
        activeColor = TFT_RED;
    } else if (strength >= 1 && strength <= 3) {
        segmentsToFill = strength; 
        activeColor = TFT_WHITE;
    } else if (strength == 4) {
        segmentsToFill = 3;        
        activeColor = TFT_GREEN;   
    } else if (strength >= 5) {
        segmentsToFill = 3;        
        activeColor = TFT_BLUE;    
    }

    sprite.drawSmoothRoundRect(x, y, 2, 2, scaledWidth, scaledHeight, bodyColor, bg_color);

    int8_t capWidth = max(1, (int)scaledWidth / 12);
    int8_t capHeight = max(2, (int)scaledHeight / 3);
    int8_t capX = x + scaledWidth;
    int8_t capY = y + (scaledHeight - capHeight) / 2;
    sprite.fillSmoothRoundRect(capX, capY, capWidth, capHeight, 1, bodyColor, bg_color);

    int8_t innerX = x + 2;
    int8_t innerY = y + 2;
    int8_t innerWidth = scaledWidth - 4;
    int8_t innerHeight = scaledHeight - 4;

    int8_t totalSegments = 3; 
    int8_t segmentGap = 1;
    int8_t totalGapsWidth = (totalSegments - 1) * segmentGap;
    int8_t segmentWidth = (innerWidth - totalGapsWidth) / totalSegments;
    if (segmentWidth < 1) segmentWidth = 1;

    for (uint8_t i = 0; i < totalSegments; i++) {
        int8_t segX = innerX + i * (segmentWidth + segmentGap);
        if (i < segmentsToFill) {
            sprite.fillSmoothRoundRect(segX, innerY, segmentWidth, innerHeight, 0, activeColor, bg_color);
        }
    }
}

void drawbackbutton(int32_t x, int32_t y, int32_t width, int32_t height, uint16_t &touchX, uint16_t &touchY, uint16_t color, uint16_t bg_color) {
    uint8_t BACK_BUTTON_CONSTANT = 201;
    
    if (touchContained(touchX, touchY, x, y, width, height)) {
        pressedButton = BACK_BUTTON_CONSTANT;
        buttonPressed(BACK_BUTTON_CONSTANT);
        color = TFT_BLUE;
    } else if (pressedButton == BACK_BUTTON_CONSTANT) {
        if (isButtonReleased(BACK_BUTTON_CONSTANT)) {
            backPage();
        }
    }

    sprite.fillRect(x, y, width, height, TFT_WHITE);
    sprite.fillTriangle(x + width / 3 + 2, y + height / 4 + 2,
                        x + width / 3 + 2, y + height * 3 / 4 - 2,
                        x + width * 2 / 3 - 2, y + height / 2,
                        color);
}

void drawToolbar(uint16_t &touchX, uint16_t &touchY, uint16_t bg_color) {
    uint8_t TOOLPAGECONSTANT = 202;
    
    drawbackbutton(0, 0, 30, 30, touchX, touchY, TFT_WHITE, bg_color);

    if (touchContained(touchX, touchY, 30, 0, 30, 210)) {
        buttonPressed(TOOLPAGECONSTANT);
    }

    if (isButtonReleased(TOOLPAGECONSTANT)) {
        pageNo = 201;
    }

    // Draws instantly using background variable tracking
    drawWifiIndicator(connectedWifiStrength, 240 - 30 - 5, 5, 20, bg_color);
    drawBatteryIndicator(getBatteryLevel(), 240 - 30 - 5 - 30, 5, 20, bg_color);
}
