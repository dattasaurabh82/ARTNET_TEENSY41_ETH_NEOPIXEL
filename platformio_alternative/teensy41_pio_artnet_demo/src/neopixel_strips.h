/**
 * @brief
 *
 */

#include <Adafruit_NeoPixel.h>

const int totalLEDStrips = 4;
// Note: Max is 170, as in what DMX allows/universe for artnet
const int numLeds = ((ledsPerStrip * numLEDStripsPerStripSocket) <= 170) ? ledsPerStrip * numLEDStripsPerStripSocket : 170;

const byte stripPins[totalLEDStrips] = {24, 25, 26, 27};
bool stripsEnabled[totalLEDStrips] = {1, 1, 0, 0};

#define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2)) ///< Transmit as G,R,B
#define NEO_KHZ800 0x0000

// Define colors.
const uint32_t RED = 0x160000;
const uint32_t GREEN = 0x001600;
const uint32_t BLUE = 0x000016;
#define totalTestColors 3
uint32_t colors[totalTestColors] = {RED, GREEN, BLUE};

Adafruit_NeoPixel strips[totalLEDStrips] = {
    Adafruit_NeoPixel(numLeds, stripPins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[3], NEO_GRB + NEO_KHZ800),
};

void clearLEDs()
{
    for (byte i = 0; i < totalLEDStrips; i++)
    {
        strips[i].clear();
        strips[i].show();
    }
}

void initStrips()
{
    for (byte i = 0; i < totalLEDStrips; i++)
    {
        strips[i].begin();
    }
    clearLEDs();
}

void initLEDTest()
{
    logln("\n---------------------------------------------------------");
    logln("NEOPIXEL STRIP/S INIT SECTION");
    logln("---------------------------------------------------------");

    if (ENABLE_OLED)
    {
        oled.clearDisplay();
        oled.setCursor(0, 0);
        oled.println("Checking LEDs...");
        oled.display();
    }
    delay(2000);

    for (byte i = 0; i < totalLEDStrips; i++)
    {
        if (stripsEnabled[i])
        {
            log("Testing Strip ");
            log(i + 1);
            log(" ... ");

            for (byte c = 0; c < totalTestColors; c++)
            {
                strips[i].fill(colors[c]);
                strips[i].show();
                delay(500);
            }
            strips[i].clear();
            strips[i].show();
            logln(" Done!");
            delay(2000);
        }
    }
}
