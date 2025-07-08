#include "ledManager.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned char LEDManager::_r = 255;
unsigned char LEDManager::_g = 0;
unsigned char LEDManager::_b = 0;
unsigned char LEDManager::_transitionType = 1;

void LEDManager::begin()
{
    strip.begin();
    strip.show();
    strip.setBrightness(50);
    _colorChase(strip.Color(_r, _b, _g), 50);
}

void LEDManager::_fadeColor(unsigned char r, unsigned char g, unsigned char b)
{

    for (int brightness = 255; brightness >= 0; brightness--)
    {
        for (int i = 0; i < LED_COUNT; i++)
        {
            strip.setPixelColor(i, strip.Color(_r * brightness / 255, _g * brightness / 255, _b * brightness / 255));
        }
        strip.show();
        delay(5);
    }

    delay(100);

    for (int brightness = 0; brightness <= 255; brightness++)
    {
        for (int i = 0; i < LED_COUNT; i++)
        {
            strip.setPixelColor(i, strip.Color(r * brightness / 255, g * brightness / 255, b * brightness / 255));
        }
        strip.show();
        delay(5);
    }
}

void LEDManager::_colorChase(uint32_t color, int wait)
{
    for (int i = 0; i < LED_COUNT; i++)
    {
        strip.setPixelColor(i, color);
        strip.show();
        delay(wait);
    }
}

void LEDManager::_setBrightness(unsigned int brightness)
{
    Serial.print("Brightness:");
    Serial.println(brightness);

    int newBrightness = constrain(brightness, 0, 255);

    strip.setBrightness(newBrightness);

    for (int i = 0; i < LED_COUNT; i++)
    {
        strip.setPixelColor(i, strip.Color(_r, _g, _b));
    }

    strip.show();
}

void LEDManager::_setColor(unsigned char red, unsigned char green, unsigned char blue)
{
    switch (_transitionType)
    {
    case 1: // fade transition
        _fadeColor(red, green, blue);
        break;
    case 2: // chasing transition
        _colorChase(strip.Color(red, green, blue), 50);
        break;
    default:
        return;
    }

    _r = red;
    _g = green;
    _b = blue;
}

void LEDManager::parsePayload(unsigned char *payload, unsigned int msg_length)
{
    StaticJsonDocument<128> cmdPayload;
    deserializeJson(cmdPayload, payload, msg_length);
    byte cmd = cmdPayload["cmd"] | 0;

    switch (cmd)
    {
    case 234:
        _setColor(cmdPayload["data"]["red"] | 255, cmdPayload["data"]["green"] | 255, cmdPayload["data"]["blue"] | 255);
        break;
    case 236:
        _setBrightness(cmdPayload["data"]["brightness"] | 50);
        break;
    case 237:
        _transitionType = cmdPayload["data"]["transitionType"] | 1;
        break;
    default:
        break;
    }
}

void LEDManager::setRGBStatus(unsigned char red, unsigned char green, unsigned char blue)
{
    _fadeColor(red,green,blue);
    _r = red;
    _g = green;
    _b = blue;
}