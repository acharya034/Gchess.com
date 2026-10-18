#include "TFTInput.h"


// ============================================================
// BUTTON
// ============================================================

TFTButton::TFTButton(TFT_eSPI& display)
    : tft(&display),
      _x(0),
      _y(0),
      _width(0),
      _height(0),
      _fontSize(1),
      _label(""),
      pressed(false),
      enabled(false),
      _theme(nullptr),
      clickTask(nullptr)
{
}


// ------------------------------------------------------------
// Set default theme
// ------------------------------------------------------------

void TFTButton::setDefaultTheme(TFTtheme& defaultTheme)
{
    _theme = &defaultTheme;
}


// ------------------------------------------------------------
// Begin button
// ------------------------------------------------------------

void TFTButton::begin(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    const String& label,
    int16_t fontSize,
    void (*task)(),
    TFTtheme* theme
)
{
    _x = x;
    _y = y;
    _width = width;
    _height = height;

    _label = label;

    _fontSize = fontSize;

    clickTask = task;

    pressed = false;
    enabled = true;


    // --------------------------------------------------------
    // IMPORTANT:
    //
    // If a temporary theme was supplied, point to it.
    //
    // If no theme was supplied, keep using the default theme
    // that was previously assigned using setDefaultTheme().
    // --------------------------------------------------------

    if (theme != nullptr) {
        _theme = theme;
    }


    // Make sure we actually have a theme.
    if (_theme != nullptr) {

        drawButton(
            _theme->buttonColor,
            _theme->textColor,
            _theme->outlineColor
        );
    }
}


// ------------------------------------------------------------
// Draw button
// ------------------------------------------------------------

void TFTButton::drawButton(
    uint16_t bcolor,
    uint16_t tcolor,
    uint16_t outlineColor
)
{
    tft->fillSmoothRoundRect(
        _x,
        _y,
        _width,
        _height,
        5,
        bcolor
    );


    tft->setTextColor(tcolor);


    // Use the requested font size.
    // tft->setTextSize(_fontSize);


    tft->setTextDatum(MC_DATUM); 
    tft->drawString(
        _label,
        _x + _width / 2,
        _y + _height / 2,
        _fontSize
    );


    tft->drawSmoothRoundRect(
        _x,
        _y,
        5,
        2,
        _width,
        _height,
        outlineColor
    );
}


// ------------------------------------------------------------
// Check touch
// ------------------------------------------------------------

bool TFTButton::contain(
    uint16_t touchX,
    uint16_t touchY
)
{
    if (!enabled)
        return false;


    if (_theme == nullptr)
        return false;


    if (
        touchX >= _x &&
        touchX <= (_x + _width) &&
        touchY >= _y &&
        touchY <= (_y + _height)
    )
    {
        // Only redraw when changing from released -> pressed.
        if (!pressed) {

            drawButton(
                _theme->pbuttonColor,
                _theme->ptextColor,
                _theme->outlineColor
            );

            pressed = true;
        }

        return true;
    }


    return false;
}


// ------------------------------------------------------------
// Action
//
// Call this when the finger/touch is released.
// ------------------------------------------------------------

void TFTButton::action()
{
    if (!enabled)
        return;


    if (!pressed)
        return;


    pressed = false;


    // Return to normal appearance.
    if (_theme != nullptr) {

        drawButton(
            _theme->buttonColor,
            _theme->textColor,
            _theme->outlineColor
        );
    }


    // Execute user's function.
    if (clickTask != nullptr) {
        clickTask();
    }
}


// ------------------------------------------------------------
// Enable
// ------------------------------------------------------------

void TFTButton::enable()
{
    enabled = true;
}


// ------------------------------------------------------------
// Disable
// ------------------------------------------------------------

void TFTButton::disable()
{
    enabled = false;
    pressed = false;
}


// ------------------------------------------------------------
// State
// ------------------------------------------------------------

bool TFTButton::isEnabled() const
{
    return enabled;
}


bool TFTButton::isPressed() const
{
    return pressed;
}



// ============================================================
// SLIDER
// ============================================================

TFTSlider::TFTSlider(TFT_eSPI& display)
    : tft(&display),
      _x(0),
      _y(0),
      _width(0),
      _height(0),
      _knobRadius(5),
      _fontSize(1),
      _upperLimit(100),
      _lowerLimit(0),
      _value(nullptr),
      _knobPosition(0),
      enabled(false),
      _theme(nullptr),
      clickTask(nullptr)
{
}


// ------------------------------------------------------------
// Set default theme
// ------------------------------------------------------------

void TFTSlider::setDefaultTheme(TFTtheme& defaultTheme)
{
    _theme = &defaultTheme;
}


// ------------------------------------------------------------
// Begin slider
// ------------------------------------------------------------

void TFTSlider::begin(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    int16_t knobRadius,
    int16_t upperLimit,
    int16_t lowerLimit,
    int16_t fontSize,
    int16_t* value,
    void (*task)(),
    TFTtheme* theme
)
{
    _x = x;
    _y = y;

    _width = width;
    _height = height;

    _knobRadius = knobRadius;

    _upperLimit = upperLimit;
    _lowerLimit = lowerLimit;

    _fontSize = fontSize;

    _value = value;

    clickTask = task;

    enabled = true;


    // --------------------------------------------------------
    // Temporary theme
    // --------------------------------------------------------

    if (theme != nullptr) {
        _theme = theme;
    }


    // --------------------------------------------------------
    // Calculate knob position.
    // --------------------------------------------------------

    updateKnobPosition();


    // --------------------------------------------------------
    // Draw slider.
    // --------------------------------------------------------

    if (_theme != nullptr) {

        drawSlider(
            _theme->buttonColor,
            _theme->outlineColor
        );
    }
}


// ------------------------------------------------------------
// Calculate knob position
// ------------------------------------------------------------

void TFTSlider::updateKnobPosition()
{
    if (_value == nullptr) {
        _knobPosition = 0;
        return;
    }


    if (_upperLimit <= _lowerLimit) {
        _knobPosition = 0;
        return;
    }


    // Keep value inside allowed range.

    if (*_value < _lowerLimit)
        *_value = _lowerLimit;

    if (*_value > _upperLimit)
        *_value = _upperLimit;


    float percentage =
        float(*_value - _lowerLimit) /
        float(_upperLimit - _lowerLimit);


    _knobPosition =
        int16_t(percentage * _width);


    // Keep position inside slider.

    if (_knobPosition < 0)
        _knobPosition = 0;

    if (_knobPosition > _width)
        _knobPosition = _width;
}


// ------------------------------------------------------------
// Draw slider
// ------------------------------------------------------------

void TFTSlider::drawSlider(
    uint16_t sliderColor,
    uint16_t outlineColor
)
{
    // Clear slider area first.
    // tft->fillRect(
    //     _x - 2,
    //     _y - 2,
    //     _width + 4,
    //     _height + 4,
    //     TFT_BLACK
    // );


    // Left/filled section.

    if (_knobPosition > 0) {

        tft->fillSmoothRoundRect(
            _x,
            _y,
            _knobPosition,
            _height,
            5,
            sliderColor
        );
    }


    // Right/unfilled section.

    if (_knobPosition < _width) {

        tft->fillSmoothRoundRect(
            _x + _knobPosition,
            _y,
            _width - _knobPosition,
            _height,
            5,
            TFT_DARKGREY
        );
    }


    // Outer border.

    tft->drawSmoothRoundRect(
        _x,
        _y,
        5,
        4,
        _width,
        _height,
        outlineColor
    );


    // Knob.

    tft->fillCircle(
        _x + _knobPosition,
        _y + (_height / 1.8),
        _knobRadius*0.8,
        sliderColor
    );

 
    tft->drawCircle(
        _x + _knobPosition,
        _y + (_height / 2),
        _knobRadius,
        outlineColor
    );
}


// ------------------------------------------------------------
// Touch slider
// ------------------------------------------------------------

bool TFTSlider::contain(
    int16_t touchX,
    int16_t touchY
)
{
    if (!enabled)
        return false;


    if (_value == nullptr)
        return false;


    if (_theme == nullptr)
        return false;


    if (
        touchX >= _x &&
        touchX <= (_x + _width) &&
        touchY >= _y -  _height / 2 &&
        touchY <= (_y + _height * 1.5)
    )
    {
        // --------------------------------------------
        // Convert touch X into slider position.
        // --------------------------------------------

        _knobPosition = touchX - _x;


        if (_knobPosition < 0)
            _knobPosition = 0;


        if (_knobPosition > _width)
            _knobPosition = _width;


        // --------------------------------------------
        // Convert position into actual value.
        // --------------------------------------------

        float percentage =
            float(_knobPosition) /
            float(_width);


        *_value =
            _lowerLimit +
            int16_t(
                percentage *
                (_upperLimit - _lowerLimit)
            );


        // --------------------------------------------
        // Redraw.
        // --------------------------------------------

        drawSlider(
            _theme->pbuttonColor,
            _theme->outlineColor
        );


        // --------------------------------------------
        // Optional callback.
        // --------------------------------------------

        if (clickTask != nullptr) {
            clickTask();
        }


        return true;
    }


    return false;
}


// ------------------------------------------------------------
// Enable
// ------------------------------------------------------------

void TFTSlider::enable()
{
    enabled = true;
}


// ------------------------------------------------------------
// Disable
// ------------------------------------------------------------

void TFTSlider::disable()
{
    enabled = false;
}


// ------------------------------------------------------------
// State
// ------------------------------------------------------------

bool TFTSlider::isEnabled() const
{
    return enabled;
}


// ------------------------------------------------------------
// Redraw
// ------------------------------------------------------------

void TFTSlider::redraw()
{
    if (_theme == nullptr)
        return;

    updateKnobPosition();

    drawSlider(
        _theme->buttonColor,
        _theme->outlineColor
    );
}


