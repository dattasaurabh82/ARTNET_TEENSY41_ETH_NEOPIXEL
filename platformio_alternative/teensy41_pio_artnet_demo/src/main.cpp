/*
   Platform: Teensy 4.1 @ 400MHz with Native Ethernet
   Repository (with HW description): https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL
   Developer/s: Saurabh Datta
   License: MIT
   About: An ARTNET based multi single strip neopixel (WS2811) system; running on Teensy 4.1 (using NativeEthernet and Artnet)
          where it receives DMX universe 0 and sets them parallel to all 4 strips
   Credits:
      1. PaulStoffregen : for the Teensy platform and initial Ethernet library    [https://github.com/PaulStoffregen/cores]
      2. Nathanaël Lécaudé : for the Artnet library                               [https://github.com/natcl/Artnet]
      3. vjmuzik: For the Native Ethernet library                                 [https://github.com/vjmuzik/NativeEthernet]
      4. Adafruit : for the Neopixel libray                                       [https://github.com/adafruit/Adafruit_NeoPixel]
      5. Adafruit : for the SSD1306 libray                                        [https://github.com/adafruit/Adafruit_SSD1306]
      6. Adafruit : for the graphics libray                                       [https://github.com/adafruit/Adafruit-GFX-Library]

  PIO specific instructions: (from scratch)
  ------------------------------------------
    1. Add manually Adafruit Busio lib from pio's library registry, to this project
    2. Add manually Adafruit GFX lib from pio's library registry, to this project
    3. Add manually Adafruit SSD1306 pio's library registry, to this project
    4. In main.cpp try importing the following libs:
        #include <Arduino.h>
        #include <SPI.h>
        #include <Wire.h>
        #include <Adafruit_GFX.h>
        #include <Adafruit_SSD1306.h>
    5. It will highlight in red complaining it couldn;t find the libariues in path.
    6. In vscode, here a small light bulb will appear, asking "Fix the quickway", do that (for both the libraries)
    7. Come back to main.cpp and compile, the errors would be gone.
    8. In platformio.ini, in "lib_deps" section, add https://github.com/vjmuzik/NativeEthernet (although is baked in Teensy System, but it might not be the latest)
    9. In platformio.ini, in "lib_deps" section, add https://github.com/natcl/Artnet, (May in pio lib registry, there is the Artnet library, but it's not latest [as of Mar 20222])

  Future Note:
  -----------
    https://forum.pjrc.com/threads/67780-Teensy-4-1-Native-Ethernet-max-SocketSize-lower-than-set?highlight=native+ethernet
    Also, I don't know when it was added), with NativeEthernet, you can use Ethernet.begin(mac, 0, 0) which does not block (even without ethernet cable)
    and takes around 5 millis to run (on T4.1). After that you can use: Ethernet.linkStatus() to know the status of the link. If no cable was inserted
    when "begin" was run, and you insert it later, a DHCP address will get assigned in non-blocking.
*/

#include <Arduino.h>

// --------------------------------------------- //
// ------- User Configurable Parameters -------- //
// --------------------------------------------- //
// Un-commenting => Enables and comment out => Disables, Serial interface for messages (e.g: for debug logs)
// #define DEBUG

// On board OLED display's parameters (for our SSD1306-128x32)
// Note: If you are using another SSD1306 screen resolution, say 128x64, then change the screen height ...
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// For neopixel on strips
const int ledsPerStrip = 144;              // change for your setup ( e.g: My 1M high-density neopixel strip has 144 LEDs ).
const byte numLEDStripsPerStripSocket = 1; // change for your setup ( e.g: I'm using 1 strip, to begin with )
// Note: Max is 170, as in what DMX allows/universe for artnet
const int numLeds = ((ledsPerStrip * numLEDStripsPerStripSocket) <= 170) ? ledsPerStrip * numLEDStripsPerStripSocket : 170;

const int channelsPerLed = 3; // (for RGB, GRB etc. it is 3 ) (for RGBW, GRBW etc. it would be 4)

// A fixed IP addres for your Teensy4.1 uC, as an Artnet node, on the network (Change it a/c to your Router settings)
byte fixedIP[] = {192, 168, 132, 150};
byte broadcast[] = {192, 168, 132, 255};
// -------------------------------------------- //

// ------------------------------------------------------------------------------------------------------------ //
// Enable/disable Serial print functionalities
// ------------------------------------------------------------------------------------------------------------ //
#ifdef DEBUG
#define log(x) Serial.print(x);
#define logln(x) Serial.println(x);
#define loglnHex(x) Serial.println(x, HEX);
#define logHex(x) Serial.print(x, HEX);
#else
#define log(x) x;
#define logln(x) x;
#define loglnHex(x) x;
#define logHex(x) x;
#endif

// ------------------------------------------------------------------------------------------------------------ //
// On-board LEDs to show, according to our logic, if network interface was succeful or not
// ------------------------------------------------------------------------------------------------------------ //
#define LED_PIN 9

void initDebugLeds()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

// --------------------------------------- //
// ----------- LIBRARY IMPORTS ----------- //
// --------------------------------------- //
#include <SPI.h>
// # [TBD] This will compile for Teensy 4.1 and Tennsy 4.0 ([TBD]: Differentiate fromTeensy 4.0 as it doesn't have Native Ethernet)
//#if defined( __IMXRT1062__)
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
// IPAddress remoteIP = {192, 168, 132, 255};
//#endif
#include <Artnet.h>

#include <Adafruit_NeoPixel.h>

#include <Wire.h>
// [TBD] for showing MAC and IP on-board, if ethernet interface was successfully initiated
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// ------------------------------------- //
// ------ SSD1306 128x32 OLED DISP------ //
// ------------------------------------- //
#define OLED_RESET_PIN 17 // Reset pin # (or -1 if sharing Arduino reset pin)
//#define OLED_SCREEN_ADDRESS 0x3C       //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
uint8_t SSD1306_ADDRESSES[2] = {0x3c, 0x3D};
uint8_t OLED_SCREEN_ADDRESS = 0x3C; //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

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

// --------------------------------------- //
// ------ WS2811 LED STRIP SETTINGS ------ //
// --------------------------------------- //
// ------------------------------------------------------------------------------------------------------------ //
// We have below 4 pins from Teensy 4.1 broken out, which can be readily connected to a ws211 neopixel strip
// ------------------------------------------------------------------------------------------------------------ //
#define totalLEDStrips 4

boolean stripsEnabled[totalLEDStrips] = {1, 1, 0, 0};

const byte stripPins[totalLEDStrips] = {24, 25, 26, 27};

#define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2)) ///< Transmit as G,R,B
#define NEO_KHZ800 0x0000                              ///< 800 KHz data transmission
Adafruit_NeoPixel strips[totalLEDStrips] = {
    Adafruit_NeoPixel(numLeds, stripPins[0], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[1], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[2], NEO_GRB + NEO_KHZ800),
    Adafruit_NeoPixel(numLeds, stripPins[3], NEO_GRB + NEO_KHZ800),
};

// Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(numLeds, stripPins[0], NEO_GRB);

// Define colors.
const uint32_t RED = 0x160000;
const uint32_t GREEN = 0x001600;
const uint32_t BLUE = 0x000016;
#define totalTestColors 3
uint32_t colors[totalTestColors] = {RED, GREEN, BLUE};

void clearLEDs()
{
  for (byte i = 0; i < totalLEDStrips; i++)
  {
    strips[i].clear();
    strips[i].show();
  }
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

void initStrips()
{
  for (byte i = 0; i < totalLEDStrips; i++)
  {
    strips[i].begin();
  }
  clearLEDs();
}

// ------------------------------------------------ //
// ------ ETHERNET SETTINGS+ ARTNET SETTINGS ------ //
// ------------------------------------------------ //
byte querryMAC[] = {0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D}; // Dummy random MAC addr used for retreiving Teensy 4.1's actual MAC addr
byte teensyMAC[6] = {};                                  // Array to hold the actual MACaddr of Teensy 4.1 (To be used for starting Ethernet Interface later)
//--
Artnet artnet;
const int startUniverse = 0;
const int numberOfChannels = numLeds * channelsPerLed; // Total number of channels you want to receive over DMX
// CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0); // Check if we got all universes...
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;
//--

void assignMAC(byte *_mac)
{
  logln("\n---------------------------------------------------------");
  logln("ETH + ARTNET INTERFACE INIT");
  logln("---------------------------------------------------------");
  logln("Getting new mac addr...");

  // oled screen text prompt
  if (ENABLE_OLED)
  {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("Trying to get new \n\nMAC ADDRESS ...");
    oled.display();
  }
  delay(2000);

  for (uint8_t by = 0; by < 2; by++)
    _mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++)
    _mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
  for (int i = 0; i < 6; i++)
  {
    teensyMAC[i] = _mac[i];
  }

#ifdef DEBUG
  Serial.printf("byte teensyMAC[] = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x };\n", teensyMAC[0], teensyMAC[1], teensyMAC[2], teensyMAC[3], teensyMAC[4], teensyMAC[5]);
#endif

  // display on OLED
  if (ENABLE_OLED)
  {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("MAC ADDR:");

    for (int i = 0; i < 6; i++)
    {
      oled.print("0x");
      if (teensyMAC[i] < 16)
      {
        oled.print(0);
      }
      oled.print(teensyMAC[i], HEX);
      oled.print(":");
    }
    oled.display();
  }
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data, IPAddress remoteIP)
{
  sendFrame = 1;
  // Set brightness ofthe whole strip, for all the strips, if they are enabled
  if (universe == 15)
  {
    for (byte i = 0; i < totalLEDStrips; i++)
    {
      if (stripsEnabled[i])
      {
        strips[i].setBrightness(data[0]);
        strips[i].show();
      }
    }
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
  {
    universesReceived[universe - startUniverse] = 1;
  }

  for (int i = 0; i < maxUniverses; i++)
  {
    if (universesReceived[i] == 0)
    {
      //      logln("Broke");
      sendFrame = 0;
      break;
    }
  }

  for (int i = 0; i < length / channelsPerLed; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / channelsPerLed);

    if (led < numLeds)
    {
      if (channelsPerLed == 4)
      {
        // -- For RGBW or GRBW type strips -- //
        for (byte s = 0; s < totalLEDStrips; s++)
        {
          // Go through all the led strips ..
          if (stripsEnabled[s])
          {
            // that are enabled..
            // and set the ccolor to incoming data
            strips[s].setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
          }
        }
      }
      if (channelsPerLed == 3)
      {
        // -- For RGB or GRB type strips -- //
        for (byte s = 0; s < totalLEDStrips; s++)
        {
          // Go through all the led strips ..
          if (stripsEnabled[s])
          {
            // that are enabled..
            // and set the ccolor to incoming data
            strips[s].setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
          }
        }
      }
    }
  }
  previousDataLength = length;

  if (sendFrame)
  {
    for (byte s = 0; s < totalLEDStrips; s++)
    {
      // Go through all the led strips ..
      if (stripsEnabled[s])
      {
        // that are enabled..
        strips[s].show();
      }
    }
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void inititateArtnet(byte _teensyMAC[], byte _fixedIP[])
{
  // void inititateArtnet() {
  //   Begin art-net with the new MAC addr and the fixed IP
  logln("Trying to begin ARTNET with Fixed IP and the above MAC addr...");
  if (ENABLE_OLED)
  {
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("Trying to begin \n\nARTNET ...");
    oled.display();
  }
  delay(2000);
  artnet.begin(_teensyMAC, _fixedIP);
  delay(2000);
  log("\nCURR IP ADDR: ");
  logln(Ethernet.localIP());
  log("ETH LINK STATUS: ");
  logln(Ethernet.linkStatus());

  /* if the fixed IP was assigned successfully, proceed; or else notify and block*/
  //  Since Artnet.begin(mac, ip) actually calls Ethernet.begin(mac, ip) underneath, we can check if our uC got the intended fixed IP address.
  //  If so, proceed.
  //  Or else, notify and block.
  IPAddress currIP = Ethernet.localIP();

  if (currIP[0] != 0 && currIP[1] != 0 && currIP[2] != 0 && currIP[3] != 0 && Ethernet.linkStatus() == 1)
  {
    logln("\nARTNET INITIATED: OK!\n");
    // Show on on-board LEDs
    digitalWrite(LED_PIN, HIGH);
    // Draw IP addr on the oled display
    if (ENABLE_OLED)
    {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.println("ARTNET: OK");
      oled.println("FIXED IP ADDR:");
      oled.println(Ethernet.localIP());
      oled.display();
    }
  }
  else
  {
    logln("\nARTNET INITIATED: FAILED [x]!\n");
    // show on on board LEDs
    digitalWrite(LED_PIN, LOW);
    // Show failed result on OLED
    if (ENABLE_OLED)
    {
      oled.clearDisplay();
      oled.setCursor(0, 0);
      oled.println("ARTNET: FAAILED!");
      oled.println("FIXED IP ADDR:");
      oled.println(Ethernet.localIP());
      oled.display();
    }
    // also if failed, do not proceed, show red on all strips & block...
    for (byte i = 0; i < totalLEDStrips; i++)
    {
      if (stripsEnabled[i])
      {
        strips[i].fill(RED);
        strips[i].show();
      }
    }
    // and block...
    while (true)
    {
      ;
    }
  }
}

// ------------------------------------------- //
// ------ MAIN SETUP FOR THE WHOLE CODE ------ //
// ------------------------------------------- //
void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
  {
    ; // wait for serial port to connect
  }
  logln(numLeds);
#endif

  // [1] Initiates on board LEDs that are used by our logic to show some status like if "network setup was successful or not..." etc.
  initDebugLeds();
  // [2] setup oled display if an old screen is attached (it will find the i2c address and initiate a display)
  setupOLEDdisplay();
  // [3] Begin & Clear the WS2811 LEDs
  initStrips();
  // [4] On init, test WS2811 LEDs
  initLEDTest();
  delay(2000);

  // [4] Get and asisgn new found MAC addr of Teensy4.1

  // Note: This is very much Teensy specific.
  // On Arduino HW platforms, we could provide any random MAC address with a fixed IP to initiate the net iface.
  // But on Teensy, we need to get the board's specific MAC address to provide it as a paramater for initiating the net iface.
  // Or else the router doesn;t assigns an IP address to the HW.
  // The below is thus a helper func which gets a Teensy board's unique MAC address and assigns it to a global variable "teensyMAC"
  assignMAC(querryMAC);
  delay(3000);

  // [5] Start the Artnet functionalities.
  inititateArtnet(teensyMAC, fixedIP);
  artnet.setBroadcast(broadcast);
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  artnet.read();
}