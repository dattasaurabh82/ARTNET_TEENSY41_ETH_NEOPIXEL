/**
 * @brief
 *
 */

// ------------------------------------------------------------------------------------------------------------ //
// Enable/disable Serial print functionalities
// ------------------------------------------------------------------------------------------------------------ //
// Un-commenting => Enables and comment out => Disables, Serial interface for messages (e.g: for debug logs)
// #define DEBUG

// ------------------------------------------------------------------------------------------------------------ //
// For SSD1306 OLED 128x32 screen related
// ------------------------------------------------------------------------------------------------------------ //
#define OLED_RESET_PIN 17                    // Reset pin # (or -1 if sharing Arduino reset pin)
uint8_t SSD1306_ADDRESSES[2] = {0x3c, 0x3D}; // LUT for ssd1306 oled displays, used to validate discovered addr.
uint8_t OLED_SCREEN_ADDRESS = 0x3C;          //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
// On board OLED display's parameters (for our SSD1306-128x32)
// Note: If you are using another SSD1306 screen resolution, say 128x64, then change the screen height ...
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// ------------------------------------------------------------------------------------------------------------ //
// ------ WS2811 LED STRIP SETTINGS ------ //
// ------------------------------------------------------------------------------------------------------------ //
#define ledsPerStrip 144                   // change for your setup ( e.g: My 1M high-density neopixel strip has 144 LEDs )
const byte numLEDStripsPerStripSocket = 1; // change for your setup ( e.g: I'm using 1 strip, to begin with )
const int channelsPerLed = 3;              // (for RGB, GRB etc. it is 3 ) (for RGBW, GRBW etc. it would be 4)

// ------------------------------------------------------------------------------------------------------------ //
// For Art net DMX library
// ------------------------------------------------------------------------------------------------------------ //
byte fixedIP[] = {192, 168, 132, 150}; // A fixed IP addres for your Teensy4.1 uC, as an Artnet node (Change it a/c to your Router settings)
byte broadcast[] = {192, 168, 132, 255};

// ------------------------------------------------------------------------------------------------------------ //
// For On-board LEDs to show, according to our logic, if network interface was succeful or not
// ------------------------------------------------------------------------------------------------------------ //
#define LED_PIN 9
