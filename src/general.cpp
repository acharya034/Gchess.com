#include "general.h"
#include <Arduino.h>
#include <TFTInput.h>

void tftInput(bool pressed, uint8_t x, uint8_t y){
    if (pressed){
        button1.contain(x, y);
        button2.contain(x, y);
        button3.contain(x, y);
        button4.contain(x, y);
        button5.contain(x, y);
        button6.contain(x, y);
        button7.contain(x, y);
        button8.contain(x, y);
        button9.contain(x, y);
        button1.contain(x, y);

        slider1.contain(x, y);
        slider2.contain(x, y);
        slider3.contain(x, y);
        slider4.contain(x, y); 

        lastPressed = true;
    }

    else if(lastPressed){
        button1.action();
        button2.action();
        button3.action();
        button4.action();
        button5.action();
        button6.action();
        button7.action();
        button8.action();
        button9.action();
        button1.action();

        lastPressed = false;
    }

    button1.disable();
    button2.disable();
    button3.disable();
    button4.disable();
    button5.disable();
    button6.disable();
    button7.disable();
    button8.disable();
    button9.disable();
    button1.disable();

    slider1.disable();
    slider2.disable();
    slider3.disable();
    slider4.disable(); 
}

// function for drawing wifi indicator on the screen.
void drawWifiIndicator(
    uint8_t strength,
    int8_t x, int8_t y,
    uint8_t size,
    uint16_t bg_color
) {
    // Map 0–255 to a drawing size, with 150 ≈ 30 pixels.
    uint16_t scaledSize = (uint16_t)size * 30 / 150;

    // Keep the minimum visible size at 1 pixel.
    if (scaledSize == 0) scaledSize = 1;

    uint16_t active   = TFT_WHITE;
    uint16_t inactive = 0x8A795D;

    int cx = x + scaledSize / 2;
    int cy = y + scaledSize - scaledSize / 8;
    int r  = scaledSize / 2;

    // Wi-Fi dot
    tft.fillSmoothCircle(
        cx, cy,
        max(1, (int)scaledSize / 16),
        (strength > 0) ? active : inactive, bg_color
    );

    // Three Wi-Fi arcs, from inner to outer
    for (uint8_t i = 0; i < 3; i++) {
        int radius = r * (i + 1) / 3;

        // Avoid zero-radius arcs at small sizes
        if (radius < 1) continue;

        uint16_t color = (i < strength) ? active : inactive;

        tft.drawSmoothArc(
            cx, cy,
            radius, radius,
            225, 315,
            color, bg_color
        );
    }
}

// Function for drawing a battery indicator with 3 blocks
void drawBatteryIndicator(uint8_t strength, int8_t x, int8_t y, uint8_t size, uint16_t bg_color) {
    // Map 0–255 to a drawing size, with 150 ≈ 30 pixels wide.
    uint16_t scaledWidth = (uint16_t)size * 30 / 150;
    if (scaledWidth < 6) scaledWidth = 6; 

    // Maintain aspect ratio (approx 2:1 width to height)
    uint16_t scaledHeight = scaledWidth * 15 / 30;
    if (scaledHeight < 4) scaledHeight = 4;

    // Color definitions
    uint16_t bodyColor = TFT_BLUE;
    uint16_t activeColor = TFT_WHITE; // Default active color is White for 1, 2, 3

    // Determine how many segments to fill and their active color
    int8_t segmentsToFill = 0;

    if (strength == 0) {
        segmentsToFill = 1;
        activeColor = TFT_RED;
    } else if (strength >= 1 && strength <= 3) {
        segmentsToFill = strength; // 1, 2, or 3 White cells
        activeColor = TFT_WHITE;
    } else if (strength == 4) {
        segmentsToFill = 3;        // All 3 cells filled
        activeColor = TFT_GREEN;   // Green for level 4
    } else if (strength >= 5) {
        segmentsToFill = 3;        // All 3 cells filled
        activeColor = TFT_BLUE;    // Blue for level 5 (Charging)
    }

    // 1. Draw Outer Battery Body Outline
    tft.drawSmoothRoundRect(x, y, 2, 2, scaledWidth, scaledHeight, bodyColor, bg_color);

    // 2. Draw Battery Terminal Cap
    int8_t capWidth = max(1, (int)scaledWidth / 12);
    int8_t capHeight = max(2, (int)scaledHeight / 3);
    int8_t capX = x + scaledWidth;
    int8_t capY = y + (scaledHeight - capHeight) / 2;
    tft.fillSmoothRoundRect(capX, capY, capWidth, capHeight, 1, bodyColor, bg_color);

    // 3. Draw Inner Cells (3 Blocks Total)
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
            // Fill the active cells with the decided color
            tft.fillSmoothRoundRect(segX, innerY, segmentWidth, innerHeight, 0, activeColor, bg_color);
        }
    }
}

