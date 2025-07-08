#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define LED_PIN    5
#define LED_COUNT 30


class LEDManager{
    public:
        static void begin();
        static void parsePayload(unsigned char* payload, unsigned int msg_length);
        static void setRGBStatus(unsigned char red, unsigned char green, unsigned char blue);
    private:
        static void _setColor(unsigned char red, unsigned char green, unsigned char blue);
        static void _setBrightness(unsigned int brightness);

        static void _fadeColor(unsigned char red, unsigned char green, unsigned char blue);
        static void _colorChase(unsigned int color, int wait);

        static unsigned char _r;
        static unsigned char _g;
        static unsigned char _b;
        static unsigned char _transitionType;
};

#endif
