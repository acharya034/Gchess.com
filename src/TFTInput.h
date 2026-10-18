#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>


// ============================================================
// THEME
// ============================================================

struct TFTtheme {
    uint16_t buttonColor;
    uint16_t textColor;
    uint16_t pbuttonColor;
    uint16_t ptextColor;
    uint16_t outlineColor;
};


// ============================================================
// BUTTON
// ============================================================

class TFTButton {

private:

    TFT_eSPI* tft;

    int16_t _x;
    int16_t _y;
    int16_t _width;
    int16_t _height;
    int16_t _fontSize;

    String _label;

    bool pressed;
    bool enabled;

    // Points to the theme being used.
    // No theme is copied.
    TFTtheme* _theme;

    // Function executed when button is released.
    void (*clickTask)();

    void drawButton(
        uint16_t bcolor,
        uint16_t tcolor,
        uint16_t outlineColor
    );

public:

    // Constructor
    TFTButton(TFT_eSPI& display);

    // Start/restart a button
    void begin(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        const String& label,
        int16_t fontSize,
        void (*task)() = nullptr,
        TFTtheme* theme = nullptr
    );

    // Set the default theme that this button should use.
    // Only changes the pointer.
    void setDefaultTheme(TFTtheme& defaultTheme);

    // Check whether a touch is inside the button.
    // Also shows pressed appearance.
    bool contain(uint16_t touchX, uint16_t touchY);

    // Call when the touch is released.
    // Executes the action if the button was pressed.
    void action();

    // Enable/disable the button.
    void enable();
    void disable();

    // Check current state.
    bool isEnabled() const;
    bool isPressed() const;
};


// ============================================================
// SLIDER
// ============================================================

class TFTSlider {

private:

    TFT_eSPI* tft;

    int16_t _x;
    int16_t _y;
    int16_t _width;
    int16_t _height;

    int16_t _knobRadius;
    int16_t _fontSize;

    int16_t _upperLimit;
    int16_t _lowerLimit;

    // User's actual value
    int16_t* _value;

    // Position of the knob in pixels
    int16_t _knobPosition;

    bool enabled;

    // Pointer to current theme.
    // No theme is copied.
    TFTtheme* _theme;

    void (*clickTask)();

    void drawSlider(
        uint16_t sliderColor,
        uint16_t outlineColor
    );

    void updateKnobPosition();

public:

    // Constructor
    TFTSlider(TFT_eSPI& display);

    // Start/restart slider.
    //
    // If theme == nullptr:
    //     use default theme.
    //
    // If theme != nullptr:
    //     use that theme directly.
    void begin(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        int16_t knobRadius,
        int16_t upperLimit,
        int16_t lowerLimit,
        int16_t fontSize,
        int16_t* value,
        void (*task)() = nullptr,
        TFTtheme* theme = nullptr
    );

    // Set the default theme pointer.
    void setDefaultTheme(TFTtheme& defaultTheme);

    // Handle touch.
    bool contain(int16_t touchX, int16_t touchY);

    // Enable/disable slider.
    void enable();
    void disable();

    bool isEnabled() const;

    // Redraw current slider.
    void redraw();
};

