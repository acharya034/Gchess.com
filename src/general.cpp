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
