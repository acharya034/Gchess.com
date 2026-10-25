#include "LichessAccount.h"

#include <ArduinoJson.h>
#include <Preferences.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <stdio.h>
#include <general.h>


// These match the QR helpers already used by the project. QRCodeData's full
// definition can stay in the existing QRCodeTFT/general header.
struct QRCodeData ;

// Define this object once in your project, for example in main.cpp:
// WebServer pairingServer(80);
extern WebServer pairingServer;

bool generateQRCode(QRCodeData*& code, const char* text,
                    uint8_t version = 4, uint8_t ecc = 1);
bool printQRCode(QRCodeData*& code, int16_t x, int16_t y, int16_t width,
                 bool printLogo = false, uint16_t backgroundColor = TFT_WHITE,
                 uint16_t codeColor = TFT_BLACK);

namespace {
constexpr char NVS_NAMESPACE[] = "lichess";
constexpr uint8_t MAX_ACCOUNTS = 250;
constexpr uint16_t MAX_TOKEN_LENGTH = 3500;
constexpr uint32_t PAIRING_SESSION_TIMEOUT = 120000UL;

volatile bool pairingServerRunning = false;
String pairingSessionId;
String completedPairingSessionId;
String pendingPairingToken;
bool pairingSessionActive = false;
bool pairingTokenStored = false;
uint32_t pairingSessionStarted = 0;
uint8_t selectedIndex = 0;
bool selectedAccountAuthenticated = false;

SemaphoreHandle_t accountMutexHandle = nullptr;

SemaphoreHandle_t accountMutex() {
    if (accountMutexHandle == nullptr) {
        // Called during initialization before normal concurrent use.
        accountMutexHandle = xSemaphoreCreateMutex();
    }
    return accountMutexHandle;
}

bool lockAccounts() {
    SemaphoreHandle_t m = accountMutex();
    return m != nullptr && xSemaphoreTake(m, portMAX_DELAY) == pdTRUE;
}

void unlockAccounts() {
    SemaphoreHandle_t m = accountMutexHandle;
    if (m != nullptr) xSemaphoreGive(m);
}

void makeKey(char prefix, uint8_t index, char* out, size_t outSize) {
    snprintf(out, outSize, "%c%u", prefix, static_cast<unsigned>(index));
}

uint8_t readCount(Preferences& prefs) {
    uint8_t count = prefs.getUChar("count", 0);
    return count > MAX_ACCOUNTS ? MAX_ACCOUNTS : count;
}

bool readAccountLocked(Preferences& prefs, uint8_t index, LichessAccount& account) {
    const uint8_t count = readCount(prefs);
    if (index == 0 || index > count) return false;

    char key[8];
    makeKey('t', index, key, sizeof(key));
    account.token = prefs.getString(key, "");
    makeKey('u', index, key, sizeof(key));
    account.username = prefs.getString(key, "");
    makeKey('r', index, key, sizeof(key));
    account.rating = prefs.getUShort(key, 0);
    makeKey('p', index, key, sizeof(key));
    account.passkey = prefs.getUShort(key, 0);

    return !account.token.isEmpty() && !account.username.isEmpty();
}

bool writeAccountLocked(Preferences& prefs, uint8_t index, const LichessAccount& account) {
    if (index == 0 || account.token.isEmpty() || account.username.isEmpty()) return false;
    if (account.token.length() > MAX_TOKEN_LENGTH) return false;

    char key[8];
    makeKey('t', index, key, sizeof(key));
    if (prefs.putString(key, account.token) != account.token.length() + 1) return false;
    makeKey('u', index, key, sizeof(key));
    if (prefs.putString(key, account.username) != account.username.length() + 1) return false;
    makeKey('r', index, key, sizeof(key));
    if (prefs.putUShort(key, account.rating) == 0) return false;
    makeKey('p', index, key, sizeof(key));
    if (prefs.putUShort(key, account.passkey) == 0) return false;
    return true;
}

bool usernameExistsLocked(Preferences& prefs, const String& username) {
    const uint8_t count = readCount(prefs);
    for (uint8_t i = 1; i <= count; ++i) {
        char key[8];
        makeKey('u', i, key, sizeof(key));
        String saved = prefs.getString(key, "");
        if (saved.equalsIgnoreCase(username)) return true;
    }
    return false;
}

uint8_t addAccountLocked(const LichessAccount& account) {
    if (account.username.isEmpty() || account.token.isEmpty() ||
        account.token.length() > MAX_TOKEN_LENGTH) return 0;

    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) return 0;
    uint8_t count = readCount(prefs);
    if (count >= MAX_ACCOUNTS || usernameExistsLocked(prefs, account.username)) {
        prefs.end();
        return 0;
    }

    const uint8_t index = count + 1;
    if (!writeAccountLocked(prefs, index, account)) {
        char key[8];
        for (char prefix : {'t', 'u', 'r', 'p'}) {
            makeKey(prefix, index, key, sizeof(key));
            prefs.remove(key);
        }
        prefs.end();
        return 0;
    }

    if (prefs.putUChar("count", index) == 0) {
        char key[8];
        for (char prefix : {'t', 'u', 'r', 'p'}) {
            makeKey(prefix, index, key, sizeof(key));
            prefs.remove(key);
        }
        prefs.end();
        return 0;
    }
    prefs.end();
    return index;
}

void addCorsHeaders() {
    pairingServer.sendHeader("Access-Control-Allow-Origin", "*");
    pairingServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    pairingServer.sendHeader("Access-Control-Allow-Headers", "Content-Type");
    pairingServer.sendHeader("Access-Control-Allow-Private-Network", "true");
    pairingServer.sendHeader("Cache-Control", "no-store");
}

void sendJson(int statusCode, JsonDocument& doc) {
    String body;
    serializeJson(doc, body);
    addCorsHeaders();
    pairingServer.send(statusCode, "application/json", body);
}

String createSessionId() {
    char id[25];
    snprintf(id, sizeof(id), "%08lx%08lx", static_cast<unsigned long>(esp_random()),
             static_cast<unsigned long>(esp_random()));
    return String(id);
}

bool sessionExpired() {
    return pairingSessionActive &&
           static_cast<uint32_t>(millis() - pairingSessionStarted) >= PAIRING_SESSION_TIMEOUT;
}

void expirePairingSession() {
    if (sessionExpired()) {
        pairingSessionActive = false;
        pairingSessionId = "";
        pendingPairingToken = "";
    }
}

void handleOptions() {
    addCorsHeaders();
    pairingServer.send(204, "text/plain", "");
}

void handleRoot() {
    JsonDocument doc;
    doc["service"] = "Gchess Lichess pairing";
    doc["ready"] = true;
    sendJson(200, doc);
}

void handlePairStart() {
    expirePairingSession();
    pairingSessionId = createSessionId();
    completedPairingSessionId = "";
    pendingPairingToken = "";
    pairingTokenStored = false;
    pairingSessionActive = true;
    pairingSessionStarted = millis();

    JsonDocument doc;
    doc["ok"] = true;
    doc["sessionId"] = pairingSessionId;
    doc["expiresInMs"] = PAIRING_SESSION_TIMEOUT;
    sendJson(200, doc);
}

void handlePairToken() {
    expirePairingSession();
    JsonDocument request;
    DeserializationError error = deserializeJson(request, pairingServer.arg("plain"));
    if (error) {
        JsonDocument response;
        response["ok"] = false;
        response["error"] = "invalid_json";
        sendJson(400, response);
        return;
    }

    const String requestedSession = request["sessionId"] | "";
    const String token = request["accessToken"] | "";
    const String username = request["username"] | "";
    const uint16_t rating = request["rating"] | 0;
    const uint16_t passkey = request["passkey"] | 0;

    if (!pairingSessionActive || requestedSession.isEmpty() ||
        requestedSession != pairingSessionId) {
        JsonDocument response;
        response["ok"] = false;
        response["error"] = "session_expired_or_invalid";
        sendJson(410, response);
        return;
    }
    if (token.isEmpty() || token.length() > MAX_TOKEN_LENGTH || username.isEmpty() ||
        username.length() > 40 || passkey > 9999) {
        JsonDocument response;
        response["ok"] = false;
        response["error"] = "missing_or_invalid_fields";
        sendJson(400, response);
        return;
    }

    LichessAccount account;
    account.token = token;
    account.username = username;
    account.rating = rating;
    account.passkey = passkey;

    uint8_t index = 0;
    if (lockAccounts()) {
        index = addAccountLocked(account);
        unlockAccounts();
    }

    if (index == 0) {
        JsonDocument response;
        response["ok"] = false;
        response["error"] = "duplicate_username_or_storage_error";
        sendJson(409, response);
        return;
    }

    // Keep the completed session ID so the browser's status poll can see success.
    completedPairingSessionId = pairingSessionId;
    pairingSessionActive = false;
    pairingSessionId = "";
    pendingPairingToken = "";
    pairingTokenStored = true;

    JsonDocument response;
    response["ok"] = true;
    response["stored"] = true;
    response["username"] = username;
    response["accountIndex"] = index;
    sendJson(200, response);
}

void handlePairStatus() {
    expirePairingSession();
    const String requested = pairingServer.arg("sessionId");
    JsonDocument response;
    if (!requested.isEmpty() && requested == completedPairingSessionId) {
        response["state"] = "connected";
        response["stored"] = true;
    } else if (pairingSessionActive && requested == pairingSessionId) {
        response["state"] = "waiting";
        response["stored"] = false;
    } else {
        response["state"] = "expired";
        response["stored"] = false;
    }
    sendJson(200, response);
}
} // namespace

bool beginLichessAccounts() {
    if (!lockAccounts()) return false;
    Preferences prefs;
    const bool ok = prefs.begin(NVS_NAMESPACE, false);
    if (ok) {
        uint8_t count = readCount(prefs);
        if (count > MAX_ACCOUNTS) prefs.putUChar("count", MAX_ACCOUNTS);
        prefs.end();
    }
    unlockAccounts();
    return ok;
}

uint8_t addAccount(const LichessAccount& account) {
    if (!lockAccounts()) return 0;
    const uint8_t index = addAccountLocked(account);
    unlockAccounts();
    return index;
}

bool getLichessToken(uint8_t index, LichessAccount& account) {
    if (!lockAccounts()) return false;
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        unlockAccounts();
        return false;
    }
    const bool ok = readAccountLocked(prefs, index, account);
    prefs.end();
    unlockAccounts();
    return ok;
}

uint8_t hasLichessToken() {
    if (!lockAccounts()) return 0;
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        unlockAccounts();
        return 0;
    }
    const uint8_t count = readCount(prefs);
    prefs.end();
    unlockAccounts();
    return count;
}

bool removeAccount(uint8_t index) {
    if (!lockAccounts()) return false;
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) {
        unlockAccounts();
        return false;
    }

    uint8_t count = readCount(prefs);
    if (index == 0) {
        prefs.clear();
        prefs.end();
        selectedIndex = 0;
        selectedAccountAuthenticated = false;
        unlockAccounts();
        return true;
    }
    if (index > count || count == 0) {
        prefs.end();
        unlockAccounts();
        return false;
    }

    char key[8];
    // Compact storage by copying each subsequent account down one index.
    for (uint8_t current = index; current < count; ++current) {
        LichessAccount next;
        if (!readAccountLocked(prefs, current + 1, next) ||
            !writeAccountLocked(prefs, current, next)) {
            prefs.end();
            unlockAccounts();
            return false;
        }
    }
    for (char prefix : {'t', 'u', 'r', 'p'}) {
        makeKey(prefix, count, key, sizeof(key));
        prefs.remove(key);
    }
    --count;
    prefs.putUChar("count", count);
    prefs.end();

    if (selectedIndex == index) {
        selectedIndex = 0;
        selectedAccountAuthenticated = false;
    } else if (selectedIndex > index) {
        --selectedIndex;
    }
    unlockAccounts();
    return true;
}

bool getAccountList(String usernames[], uint8_t maxItems, uint8_t& count) {
    count = 0;
    if (usernames == nullptr || maxItems == 0 || !lockAccounts()) return false;
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        unlockAccounts();
        return false;
    }
    const uint8_t storedCount = readCount(prefs);
    for (uint8_t i = 1; i <= storedCount && count < maxItems; ++i) {
        char key[8];
        makeKey('u', i, key, sizeof(key));
        String username = prefs.getString(key, "");
        if (!username.isEmpty()) usernames[count++] = username;
    }
    prefs.end();
    unlockAccounts();
    return true;
}

bool selectAccount(uint8_t index) {
    LichessAccount account;
    if (!getLichessToken(index, account)) return false;
    if (!lockAccounts()) return false;
    selectedIndex = index;
    selectedAccountAuthenticated = account.passkey == 0;
    unlockAccounts();
    return true;
}

bool checkAccountPasskey(uint16_t enteredPasskey) {
    if (!lockAccounts()) return false;
    if (selectedIndex == 0) {
        unlockAccounts();
        return false;
    }
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, true)) {
        unlockAccounts();
        return false;
    }
    char key[8];
    makeKey('p', selectedIndex, key, sizeof(key));
    const uint16_t savedPasskey = prefs.getUShort(key, 0);
    prefs.end();
    const bool ok = savedPasskey == 0 || savedPasskey == enteredPasskey;
    selectedAccountAuthenticated = ok;
    unlockAccounts();
    return ok;
}

bool isAccountSelected() {
    if (!lockAccounts()) return false;
    const bool selected = selectedIndex != 0;
    unlockAccounts();
    return selected;
}

void deselectAccount() {
    if (!lockAccounts()) return;
    selectedIndex = 0;
    selectedAccountAuthenticated = false;
    unlockAccounts();
}

String getAccountUsername() {
    LichessAccount account;
    if (!lockAccounts()) return String();
    const uint8_t index = selectedIndex;
    const bool authenticated = selectedAccountAuthenticated;
    unlockAccounts();
    if (index == 0 || !authenticated || !getLichessToken(index, account)) return String();
    return account.username;
}

uint16_t getAccountRating() {
    LichessAccount account;
    if (!lockAccounts()) return 0;
    const uint8_t index = selectedIndex;
    const bool authenticated = selectedAccountAuthenticated;
    unlockAccounts();
    if (index == 0 || !authenticated || !getLichessToken(index, account)) return 0;
    return account.rating;
}

String getAccountToken(){
    LichessAccount account;
    if (!lockAccounts()) return "";
    const uint8_t index = selectedIndex;
    const bool authenticated = selectedAccountAuthenticated;
    unlockAccounts();
    if (index == 0 || !authenticated || !getLichessToken(index, account)) return "";
    return account.token;
}

bool getAccountToken(String& token) {
    token = getAccountToken();
    if (token.length() > 0) return true;
    return false;
}

bool updateAccountRating(uint16_t rating) {
    if (!lockAccounts()) return false;
    const uint8_t index = selectedIndex;
    if (index == 0) {
        unlockAccounts();
        return false;
    }
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) {
        unlockAccounts();
        return false;
    }
    char key[8];
    makeKey('r', index, key, sizeof(key));
    const uint16_t current = prefs.getUShort(key, 0);
    const bool ok = current == rating || prefs.putUShort(key, rating) != 0;
    prefs.end();
    unlockAccounts();
    return ok;
}

bool updateAccountPasskey(uint16_t passkey) {
    if (passkey > 9999 || !lockAccounts()) return false;
    const uint8_t index = selectedIndex;
    if (index == 0) {
        unlockAccounts();
        return false;
    }
    Preferences prefs;
    if (!prefs.begin(NVS_NAMESPACE, false)) {
        unlockAccounts();
        return false;
    }
    char key[8];
    makeKey('p', index, key, sizeof(key));
    const uint16_t current = prefs.getUShort(key, 0);
    const bool ok = current == passkey || prefs.putUShort(key, passkey) != 0;
    if (ok) selectedAccountAuthenticated = passkey == 0;
    prefs.end();
    unlockAccounts();
    return ok;
}

void beginLichessPairingServer() {
    pairingTokenStored = false;
    pairingServerRunning = true;
    pairingServer.on("/", HTTP_GET, handleRoot);
    pairingServer.on("/pair/start", HTTP_OPTIONS, handleOptions);
    pairingServer.on("/pair/token", HTTP_OPTIONS, handleOptions);
    pairingServer.on("/pair/status", HTTP_OPTIONS, handleOptions);
    pairingServer.on("/pair/start", HTTP_POST, handlePairStart);
    pairingServer.on("/pair/token", HTTP_POST, handlePairToken);
    pairingServer.on("/pair/status", HTTP_GET, handlePairStatus);
    pairingServer.onNotFound([]() {
        addCorsHeaders();
        pairingServer.send(404, "application/json", "{\"error\":\"not_found\"}");
    });
    pairingServer.begin();
}

bool runLichessPairing() {
    beginLichessPairingServer();
    while (pairingServerRunning) {
        pairingServer.handleClient();
        expirePairingSession();
        delay(2);
    }
    pairingServer.stop();
    return pairingTokenStored;
}

void stopLichessPairing() {
    pairingServerRunning = false;
}

bool printLichessPairingQR(QRCodeData*& code, int16_t x, int16_t y,
                           int16_t width, bool printLogo) {
    if (WiFi.status() != WL_CONNECTED) return false;
    String url = "https://acharya034.github.io/Gchess-Server/?device=" + WiFi.localIP().toString();
    if (!generateQRCode(code, url.c_str())) return false;
    return printQRCode(code, x, y, width, printLogo, TFT_WHITE, TFT_BLACK);
}
