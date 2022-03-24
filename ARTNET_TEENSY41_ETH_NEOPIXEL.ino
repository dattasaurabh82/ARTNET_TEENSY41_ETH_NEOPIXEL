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
*/

// https://forum.pjrc.com/threads/67780-Teensy-4-1-Native-Ethernet-max-SocketSize-lower-than-set?highlight=native+ethernet
// Also, you can use Ethernet.begin(mac,0,0) which does not block (even without ethernet cable) and takes around 5 millis to run (on T4.1).
// After that you can use Ethernet.linkStatus() to know the status of the link.
// If no cable was inserted when "begin" was run, and you insert it later, a DHCP address will get assigned in non-blocking.

/*
   TODO:
   [-] create array of strips and not individual objects?
   [*] dmx callack to all or 1 strip.
   [-] check multiple dmx universe assignment
   [*] i2c OLED display setup 
   [-] Check network once in a while (make it hot swappable) 
   [-] Button press + interrupt.
*/

// ---------------------------------------------- //
// ------- User Configurable Parameters --------- //
// ---------------------------------------------- //
// Un-commenting => Enables and comment out => Disables, Serial interface for messages (e.g: for debug logs)
#define DEBUG

// On board OLED display's parameters (for our SSD1306-128x32)
// Note: If you are using another SSD1306 screen resolution, say 128x64, then change the screen height ...
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// Enable or Disable strips during pre-compilation already by un-commenting or commenting (we have 4 on our PCB)
// #define ENABLE_STRIP1
// #define ENABLE_STRIP2
#define ENABLE_STRIP3
#define ENABLE_STRIP4

// A fixed IP addres for your Teensy4.1 uC, as an Artnet node, on the network (Change it a/c to your Router settings)
byte fixedIP[] = { 192, 168, 132, 150 };
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
// On-board Push Buttons (which are placed next to each strip sockts)
// ------------------------------------------------------------------------------------------------------------ //
const byte stripBtnPins[4] = { 26, 27, 41, 40 };

void initiateOnBoardPushButtons() {
  for (int i = 0; i < sizeof(stripBtnPins); i++) {
    pinMode(stripBtnPins[i], INPUT_PULLUP);
  }
}

// ------------------------------------------------------------------------------------------------------------ //
// On-board LEDs to show, according to our logic, if network interface was succeful or not
// ------------------------------------------------------------------------------------------------------------ //
#define NET_INIT_SUCCESS_LED_PIN 17
#define NET_INIT_FAIL_LED_PIN 16

void initDebugLeds() {
  pinMode(NET_INIT_SUCCESS_LED_PIN, OUTPUT);
  pinMode(NET_INIT_FAIL_LED_PIN, OUTPUT);
  digitalWrite(NET_INIT_SUCCESS_LED_PIN, LOW);
  digitalWrite(NET_INIT_FAIL_LED_PIN, LOW);
}



// --------------------------------------- //
// ----------- LIBRARY IMPORTS ----------- //
// --------------------------------------- //
#include <SPI.h>
// # This will compile for Teensy 4.1 and Tennsy 4.0 ([TBD]: Differentiate fromTeensy 4.0 as it doesn't have Native Ethernet)
//#if defined( __IMXRT1062__)
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
//#endif
#include <Artnet.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
// [TBD] for showing MAC and IP on-board, if ethernet interface was successfully initiated
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <U8g2lib.h>


// ------------------------------------- //
// ------ SSD1306 128x32 OLED DISP------ //
// ------------------------------------- //
#define OLED_RESET_PIN  4                // Reset pin # (or -1 if sharing Arduino reset pin)
//#define OLED_SCREEN_ADDRESS 0x3C       //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
uint8_t OLED_SCREEN_ADDRESS = 0x3C;      //< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// --- OLED display's i2c addr auto-discovery feature --- //
uint8_t getDisplayAddr() {
  byte error;
  byte address;
  byte foundAddr;
  int nDevices = 0;

  // Buffer time to let the device initialize
  delay(500);

  for (address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      foundAddr = address;

      logln("Found a i2c slave!");
      nDevices++;
    } else if (error == 4) {
      foundAddr = 0;
      // foundAddr = 0x00;

      log("Error code: ");
      logln(error);
      log(F("Unknown error at address 0x"));
      logHex(foundAddr);
      logln("\n");
    }
  }

  if (nDevices == 0) {
    logln(F("No I2C devices found\n"));
  }
  log("i2C SLAVE'S ADDR: [ Hex: 0x"); logHex(foundAddr); log(", Binary: "); log(foundAddr); logln("]");
  return foundAddr;
}

Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);



// --------------------------------------- //
// ------ WS2811 LED STRIP SETTINGS ------ //
// --------------------------------------- //
// ------------------------------------------------------------------------------------------------------------ //
// We have below 4 pins from Teensy 4.1 broken out, which can be readily connected to a ws211 neopixel strip
// ------------------------------------------------------------------------------------------------------------ //
// const byte stripPins[4] = { Strip 1 pin, Strip 2 pin, Strip 3 pin, Strip 4 pin };
const byte stripPins[4] = { 24, 25, 15, 14 };
// ** Note: of-course, you can use any other digital pin at their disposal (for example, if not using our dev PCB)

// -- For Neopixel lib -- //
const int ledsPerStrip = 144;                               // change for your setup ( e.g: My 1M high-density neopixel strip has 144 LEDs )
const byte numStrips = 1;                                   // change for your setup ( e.g: I'm using 1 strip, to begin with )
const int numLeds = ledsPerStrip * numStrips;
const int channelsPerLed = 3;                               // (for RGB, GRB etc. it is 3 ) (for RGBW, GRBW etc. it would be 4)
const int numberOfChannels = numLeds * channelsPerLed;      // Total number of channels you want to receive
byte channelBuffer[numberOfChannels]; // Combined universes into a single arra09

#ifdef ENABLE_STRIP1
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(numLeds, stripPins[0], NEO_GRB + NEO_KHZ800);
#endif
#ifdef ENABLE_STRIP2
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(numLeds, stripPins[1], NEO_GRB + NEO_KHZ800);
#endif
#ifdef ENABLE_STRIP3
Adafruit_NeoPixel strip3 = Adafruit_NeoPixel(numLeds, stripPins[2], NEO_GRB + NEO_KHZ800);
#endif
#ifdef ENABLE_STRIP4
Adafruit_NeoPixel strip4 = Adafruit_NeoPixel(numLeds, stripPins[3], NEO_GRB + NEO_KHZ800);
#endif

// Define colors.
#ifdef ENABLE_STRIP1
uint32_t RED1 = strip1.Color(127, 0, 0);
uint32_t GREEN1 = strip1.Color(0, 127, 0);
uint32_t BLUE1 =  strip1.Color(0, 0, 127);
#endif
#ifdef ENABLE_STRIP2
uint32_t RED2 = strip2.Color(127, 0, 0);
uint32_t GREEN2 = strip2.Color(0, 127, 0);
uint32_t BLUE2 =  strip2.Color(0, 0, 127);
#endif
#ifdef ENABLE_STRIP3
uint32_t RED3 = strip3.Color(127, 0, 0);
uint32_t GREEN3 = strip3.Color(0, 127, 0);
uint32_t BLUE3 =  strip3.Color(0, 0, 127);
#endif
#ifdef ENABLE_STRIP4
uint32_t RED4 = strip4.Color(127, 0, 0);
uint32_t GREEN4 = strip4.Color(0, 127, 0);
uint32_t BLUE4 =  strip4.Color(0, 0, 127);
#endif






void clearLEDs() {
#ifdef ENABLE_STRIP1
  strip1.clear();
  strip1.show();
#endif
#ifdef ENABLE_STRIP2
  strip2.clear();
  strip2.show();
#endif
#ifdef ENABLE_STRIP3
  strip3.clear();
  strip3.show();
#endif
#ifdef ENABLE_STRIP4
  strip4.clear();
  strip4.show();
#endif
}

void initLEDTest() {
#ifdef ENABLE_STRIP1
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Checking Strip 1\n...");
  oled.display();

  log("Testing Strip 1... ");

  strip1.fill(RED1);
  strip1.show();
  delay(500);
  strip1.fill(GREEN1);
  strip1.show();
  delay(500);
  strip1.fill(BLUE1);
  strip1.show();
  delay(500);
  strip1.clear();
  strip1.show();


  oled.println("Done!");
  oled.display();

  logln("Done!");

  delay(3000);
#endif

#ifdef ENABLE_STRIP2
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Checking Strip 2\n...");
  oled.display();

  log("Testing Strip 2... ");

  strip2.fill(RED2);
  strip2.show();
  delay(500);
  strip2.fill(GREEN2);
  strip2.show();
  delay(500);
  strip2.fill(BLUE2);
  strip2.show();
  delay(500);
  strip2.clear();
  strip2.show();

  oled.println("Done!");
  oled.display();

  logln("Done!");

  delay(3000);
#endif

#ifdef ENABLE_STRIP3
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Checking Strip 3\n...");
  oled.display();

  log("Testing Strip 3... ");

  strip3.fill(RED3);
  strip3.show();
  delay(500);
  strip3.fill(GREEN3);
  strip3.show();
  delay(500);
  strip3.fill(BLUE3);
  strip3.show();
  delay(500);
  strip3.clear();
  strip3.show();

  oled.println("Done!");
  oled.display();

  logln("Done!");

  delay(3000);
#endif

#ifdef ENABLE_STRIP4
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Checking Strip 4\n...");
  oled.display();

  log("Testing Strip 4... ");

  strip4.fill(RED4);
  strip4.show();
  delay(500);
  strip4.fill(GREEN4);
  strip4.show();
  delay(500);
  strip4.fill(BLUE4);
  strip4.show();
  delay(500);
  strip4.clear();
  strip4.show();

  oled.println("Done!");
  oled.display();

  logln("Done!\n");
#endif
}



// ------------------------------- //
// ------- ARTNET SETTINGS ------- //
// ------------------------------- //
Artnet artnet;
const int startUniverse = 0;                                                          // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0); // Check if we got all universes...
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;




void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data) {
  sendFrame = 1;
  // Set brightness of the whole strip
  if (universe == 15) {
#ifdef ENABLE_STRIP1
    strip1.setBrightness(data[0]);
    strip1.show();
#endif
#ifdef ENABLE_STRIP2
    strip2.setBrightness(data[0]);
    strip2.show();
#endif
#ifdef ENABLE_STRIP3
    strip3.setBrightness(data[0]);
    strip3.show();
#endif
#ifdef ENABLE_STRIP4
    strip4.setBrightness(data[0]);
    strip4.show();
#endif
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses) {
    universesReceived[universe - startUniverse] = 1;
  }


  for (int i = 0 ; i < maxUniverses ; i++) {
    if (universesReceived[i] == 0) {
      // logln("Broke");
      sendFrame = 0;
      break;
    }
  }


  for (int i = 0; i < length / channelsPerLed; i++) {
    int led = i + (universe - startUniverse) * (previousDataLength / channelsPerLed);

    if (led < numLeds) {
      if (channelsPerLed == 4) {
        // For RGBW or GRBW type strips
#ifdef ENABLE_STRIP1
        strip1.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
#endif
#ifdef ENABLE_STRIP2
        strip2.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
#endif
#ifdef ENABLE_STRIP3
        strip3.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
#endif
#ifdef ENABLE_STRIP4
        strip4.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2], data[i * channelsPerLed + 3]);
#endif
      }
      if (channelsPerLed == 3) {
        // For RGB or GRB type strips
#ifdef ENABLE_STRIP1
        strip1.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
#endif
#ifdef ENABLE_STRIP2
        strip2.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
#endif
#ifdef ENABLE_STRIP3
        strip3.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
#endif
#ifdef ENABLE_STRIP4
        strip4.setPixelColor(led, data[i * channelsPerLed], data[i * channelsPerLed + 1], data[i * channelsPerLed + 2]);
#endif
      }
    }
  }
  previousDataLength = length;

  if (sendFrame) {
#ifdef ENABLE_STRIP1
    strip1.show();
#endif
#ifdef ENABLE_STRIP2
    strip2.show();
#endif
#ifdef ENABLE_STRIP3
    strip3.show();
#endif
#ifdef ENABLE_STRIP4
    strip4.show();
#endif
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}




// ------------------------------- //
// ------ ETHERNET SETTINGS ------ //
// ------------------------------- //
byte querryMAC[] = { 0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D }; // Dummy random MAC addr used for retreiving Teensy 4.1's actual MAC addr
byte teensyMAC[6] = {};                                    // Array to hold the actual MACaddr of Teensy 4.1 (To be used for starting Ethernet Interface later)


void assignMAC(byte * _mac) {
  for (uint8_t by = 0; by < 2; by++) _mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++) _mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
  for (int i = 0; i < 6; i++) {
    teensyMAC[i] = _mac[i];
  }
}





// ------------------------------------------- //
// ------ MAIN SETUP FOR THE WHOLE CODE ------ //
// ------------------------------------------- //
void setup() {
#ifdef DEBUG
  while (!Serial) {
    ; // wait for serial port to connect
  }
#endif

  // Initiates on board LEDs that are used by our logic to show some status like if "network setup was successful or not..." etc.
  initDebugLeds();

  logln("---------------------------------------------------------");
  logln("OLED SCREEN INIT SECTION");
  logln("---------------------------------------------------------");
  Wire.begin();
  OLED_SCREEN_ADDRESS = getDisplayAddr();
  Wire.end();

  //[TBD] if oled screen address is not from the known look up table...

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    logln(F("\nOLED SSD1306 INITIATED: FAILED [x]\n"));
  } else {
    oled.setFont();                                       // default font
    // oled was successfully initiated, so Clear the OLED buffer
    oled.clearDisplay();
    oled.display();
    oled.setTextSize(1);                                  // Normal 1:1 pixel scale
    oled.setTextColor(SSD1306_WHITE);                     // Draw white text
    oled.setCursor(0, 0);                                 // Start at top-left corner

    logln("\nOLED SSD1306 INITIATED: OK!\n");
  }





  logln("\n---------------------------------------------------------");
  logln("NEIOPIXEL STRIP/S INIT SECTION");
  logln("---------------------------------------------------------");

  // Begin & Clear the WS2811 LEDs
#ifdef ENABLE_STRIP1
  strip1.begin();
#endif
#ifdef ENABLE_STRIP2
  strip2.begin();
#endif
#ifdef ENABLE_STRIP3
  strip3.begin();
#endif
#ifdef ENABLE_STRIP4
  strip4.begin();
#endif

  // [TBD] Set brightness of ws2812 LEDs here may be ...

  clearLEDs();

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Checking LEDs...");
  oled.display();

  delay(3000);

  // On init, test WS2811 LEDs
  initLEDTest();

  delay(3000);

  //  Get and asisgn new found MAC addr of Teensy4.1
  logln("\n---------------------------------------------------------");
  logln("ETH + ARTNET INTERFACE INIT");
  logln("---------------------------------------------------------");
  logln("Getting new mac addr...");
  // oled screen text prompt
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Trying to get new \n\nMAC ADDRESS ...");
  oled.display();

  delay(3000);

  assignMAC(querryMAC);

#ifdef DEBUG
  Serial.printf("byte teensyMAC[] = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x };\n", teensyMAC[0], teensyMAC[1], teensyMAC[2], teensyMAC[3], teensyMAC[4], teensyMAC[5]);
#endif

  // display on OLED
  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("MAC ADDR:");
  for (int i = 0; i < sizeof(teensyMAC); i++) {
    oled.print("0x");
    if (teensyMAC[i] < 16) {
      oled.print(0);
    }
    oled.print(teensyMAC[i], HEX);
    oled.print(":");
  }
  oled.display();

  delay(3000);


  //  Begin art-net with the new MAC addr and the fixed IP
  logln("Trying to begin ARTNET with Fixed IP and the above MAC addr...");

  oled.clearDisplay();
  oled.setCursor(0, 0);
  oled.println("Trying to begin \n\nARTNET ...");
  oled.display();

  delay(2000);
  artnet.begin(teensyMAC, fixedIP);
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

  if (currIP[0] != 0 && currIP[1] != 0 && currIP[2] != 0 && currIP[3] != 0 && Ethernet.linkStatus() == 1) {
    logln("\nARTNET INITIATED: OK!\n");

    // Show on on board LEDs
    digitalWrite(NET_INIT_SUCCESS_LED_PIN, HIGH);
    digitalWrite(NET_INIT_FAIL_LED_PIN, LOW);

    // Draw IP addr on the oled display
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("ARTNET: OK");
    oled.println("FIXED IP ADDR:");
    oled.println(Ethernet.localIP());
    oled.display();
  } else {
    logln("\nARTNET INITIATED: FAILED [x]!\n");

    // show on on board LEDs
    digitalWrite(NET_INIT_SUCCESS_LED_PIN, LOW);
    digitalWrite(NET_INIT_FAIL_LED_PIN, HIGH);

    // Show failed result on OLED
    oled.clearDisplay();
    oled.setCursor(0, 0);
    oled.println("ARTNET: FAAILED!");
    oled.println("FIXED IP ADDR:");
    oled.println(Ethernet.localIP());
    oled.display();
    // also if failed, do not proceed, show red on all strips & block...
#ifdef ENABLE_STRIP1
    strip1.fill(RED1);
    strip1.show();
#endif
#ifdef ENABLE_STRIP2
    strip2.fill(RED2);
    strip2.show();
#endif
#ifdef ENABLE_STRIP3
    strip3.fill(RED3);
    strip3.show();
#endif
#ifdef ENABLE_STRIP4
    strip4.fill(RED4);
    strip4.show();
#endif
    while (true) {
      ;
    }
  }

  artnet.setBroadcast(broadcast);
  artnet.setArtDmxCallback(onDmxFrame);
}



// --------------------------- //
// ------ INFINITE LOOP ------ //
// --------------------------- //
void loop() {
  artnet.read();
}
