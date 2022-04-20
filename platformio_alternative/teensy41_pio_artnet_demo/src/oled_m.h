/**
 * @brief
 * For showing MAC and IP on-board, if ethernet interface was successfully initiated
 */

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// --- OLED display's i2c addr auto-discovery feature --- //
uint8_t getDisplayAddr()
{
    uint8_t error = 1;
    uint8_t address = 0x00;
    uint8_t foundAddr = 0x00;
    int nDevices = 0;

    // Buffer time to let the device initialize
    delay(100);

    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        if (error == 0)
        {
            foundAddr = address;
            logln("Found a i2c slave!");
            nDevices++;
            break;
        }
        else if (error == 4)
        {
            log("Error code: ");
            logln(error);
            log(F("Unknown error at address 0x"));
            logHex(foundAddr);
            logln("\n");
            break;
        }
    }

    if (nDevices == 0)
    {
        logln("No I2C devices found\n");
    }
    else
    {
        log("i2C SLAVE'S ADDR: [ Hex: 0x");
        logHex(foundAddr);
        log(", Binary: ");
        log(foundAddr);
        logln("]");
    }
    return foundAddr;
}

boolean validAddr(uint8_t _addr)
{
    bool isValidAddr = false;
    for (byte i = 0; i < sizeof(SSD1306_ADDRESSES); i++)
    {
        if (_addr == SSD1306_ADDRESSES[i])
        {
            isValidAddr = true;
            log("ADDR ");
            logHex(_addr);
            logln(" found in LUT");

            break;
        }
        else
        {
            isValidAddr = false;
        }
    }
    return isValidAddr;
}

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
boolean ENABLE_OLED = false;

void setupOLEDdisplay()
{
    logln("---------------------------------------------------------");
    logln("OLED SCREEN INIT SECTION");
    logln("---------------------------------------------------------");
    Wire.begin();
    OLED_SCREEN_ADDRESS = getDisplayAddr();
    Wire.end();
    if (validAddr(OLED_SCREEN_ADDRESS))
    {
        // #define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V
        if (oled.begin(0x02C, OLED_SCREEN_ADDRESS))
        {
            ENABLE_OLED = true;
            logln("\nOLED SSD1306 INITIATED: OK!\n");
        }
        else
        {
            ENABLE_OLED = false;
            logln(F("\nOLED SSD1306 INITIATED: FAILED [x]\n"));
        }
    }
    else
    {
        ENABLE_OLED = false;
        logln(F("\nOLED SSD1306 INITIATED: FAILED [x]\n"));
    }
    if (ENABLE_OLED)
    {
        // SETUP OLED
        // More details:   https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
        oled.setFont(); // default font
        // oled was successfully initiated, so Clear the OLED buffer
        oled.clearDisplay();
        oled.display();
        oled.setTextSize(1); // Normal 1:1 pixel scale
        // oled.setTextColor(SSD1306_WHITE); // Draw white text
        // #define SSD1306_WHITE 1               ///< Draw 'on' pixels
        oled.setTextColor(1); // Draw white text
        oled.setCursor(0, 0); // Start at top-left corner

        // HELLO WORLD MESSAGE
        oled.clearDisplay();
        oled.setCursor(0, 0);
        oled.println("Hello World!");
        oled.display();
    }

    // .. for test:
    // ENABLE_OLED = false;
}