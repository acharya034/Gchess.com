#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <general.h>

// QRCodeData is defined by your existing QRCodeTFT/general header.
struct LichessAccount {
    String token;
    String username;
    uint16_t rating = 0;
    uint16_t passkey = 0; // 0 means no passkey
};

// Account storage. Account indexes are 1..count; index 0 means all accounts
// only for removeAccount(0).
bool beginLichessAccounts();
uint8_t addAccount(const LichessAccount& account); // returns index, or 0 on failure
bool getLichessToken(uint8_t index, LichessAccount& account);
uint8_t hasLichessToken(); // returns the number of stored accounts
bool removeAccount(uint8_t index = 0); // 0 clears all accounts; otherwise compacts indexes

bool getAccountList(String usernames[], uint8_t maxItems, uint8_t& count);

// Selected-account helpers. Selection is held in RAM, not persisted across reboot.
bool selectAccount(uint8_t index);
bool checkAccountPasskey(uint16_t enteredPasskey);
bool isAccountSelected();
void deselectAccount();
String getAccountUsername();
uint16_t getAccountRating();
bool getAccountToken(String& token);
String getAccountToken();

// Returns true if the stored value was already equal or was successfully updated.
bool updateAccountRating(uint16_t rating);
bool updateAccountPasskey(uint16_t passkey);

// Pairing server functions. Wi-Fi must already be connected.
void beginLichessPairingServer();
bool runLichessPairing(); // blocking loop; normally run on Core 1
void stopLichessPairing(); // safe to request from the UI/Core 0
bool printLichessPairingQR(QRCodeData*& code, int16_t x, int16_t y,
                           int16_t width, bool printLogo = false);
