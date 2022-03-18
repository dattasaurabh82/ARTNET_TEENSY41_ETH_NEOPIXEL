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
*/

// https://forum.pjrc.com/threads/67780-Teensy-4-1-Native-Ethernet-max-SocketSize-lower-than-set?highlight=native+ethernet
// Also, you can use Ethernet.begin(mac,0,0) which does not block (even without ethernet cable) and takes around 5 millis to run (on T4.1).
// After that you can use Ethernet.linkStatus() to know the status of the link.
// If no cable was inserted when "begin" was run, and you insert it later, a DHCP address will get assigned in non-blocking.

/*
   TODO:
   [] ARTNET Broadcast functionality.
   [] Button press + interrupt.
   [] dmx callack to all or 1 strip.
   [] i2c OLED display + interrupt.
*/

// ------------------------------------------------------------------------------------------------------------ //
// Un-commenting => Enables and comment out => Disables Serial interface for messages (e.g: for debug logs)
// ------------------------------------------------------------------------------------------------------------ //
// #define DEBUG

#ifdef DEBUG
#define log(x) Serial.print(x);
#define logln(x) Serial.println(x);
#else
#define log(x) x;
#define logln(x) x;
#endif



// ---------------------------------------------- //
// ------ ** SETUPS SPECIFIC TO OUR PCB ** ------ //
// ---------------------------------------------- //

// ------------------------------------------------------------------------------------------------------------ //
// Enable or Disable strips (we have 4 on our PCB) by un-commenting or commenting out for respective reasons
// ------------------------------------------------------------------------------------------------------------ //
//#define ENABLE_STRIP1
//#define ENABLE_STRIP2
#define ENABLE_STRIP3
#define ENABLE_STRIP4

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

// ------------------------------------------------------------------------------------------------------------ //
// We have below 4 pins from Teensy 4.1 broken out, which can be readily connected to a ws211 neopixel strip
// ------------------------------------------------------------------------------------------------------------ //
// const byte stripPins[4] = { Strip 1 pin, Strip 2 pin, Strip 3 pin, Strip 4 pin };
const byte stripPins[4] = { 24, 25, 15, 14 };
// ** Note: of-course, you can use any other digital pin at their disposal (for example, if not using our dev PCB)




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


// --------------------------------------- //
// ------ WS2811 LED STRIP SETTINGS ------ //
// --------------------------------------- //
const int ledsPerStrip = 144;                               // change for your setup ( e.g: My 1M high-density neopixel strip has 144 LEDs )
const byte numStrips = 1;                                   // change for your setup ( e.g: I'm using 1 strip, to begin with )
const int numLeds = ledsPerStrip * numStrips;
const int channelsPerLed = 3;                               // (for RGB, GRB etc. it is 3 ) (for RGBW, GRBW etc. it would be 4)
const int numberOfChannels = numLeds * channelsPerLed;      // Total number of channels you want to receive

// [TBD] ...
//Adafruit_NeoPixel strips[4];

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
  log("Testing Strip 1... ");
  strip1.fill(RED1);
  strip1.show();
  delay(1000);
  strip1.fill(GREEN1);
  strip1.show();
  delay(1000);
  strip1.fill(BLUE1);
  strip1.show();
  delay(1000);
  strip1.clear();
  strip1.show();
  logln("Done!");
  delay(1000);
#endif

#ifdef ENABLE_STRIP2
  log("Testing Strip 2... ");
  strip2.fill(RED2);
  strip2.show();
  delay(1000);
  strip2.fill(GREEN2);
  strip2.show();
  delay(1000);
  strip2.fill(BLUE2);
  strip2.show();
  delay(1000);
  strip2.clear();
  strip2.show();
  logln("Done!");
  delay(1000);
#endif

#ifdef ENABLE_STRIP3
  log("Testing Strip 3... ");
  strip3.fill(RED3);
  strip3.show();
  delay(1000);
  strip3.fill(GREEN3);
  strip3.show();
  delay(1000);
  strip3.fill(BLUE3);
  strip3.show();
  delay(1000);
  strip3.clear();
  strip3.show();
  logln("Done!");
  delay(1000);
#endif

#ifdef ENABLE_STRIP4
  log("Testing Strip 4... ");
  strip4.fill(RED4);
  strip4.show();
  delay(1000);
  strip4.fill(GREEN4);
  strip4.show();
  delay(1000);
  strip4.fill(BLUE4);
  strip4.show();
  delay(1000);
  strip4.clear();
  strip4.show();
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
    strip3.setBrightness(data[0]);
    strip3.show();
  }

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0 ; i < maxUniverses ; i++) {
    if (universesReceived[i] == 0) {
      // logln("Broke");
      sendFrame = 0;
      break;
    }
  }

  // Read universe and put into the right part of the display buffer
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
    strip3.show();
    strip4.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}


//void onSync() {
//  strip3.show();
//}






// ------------------------------- //
// ------ ETHERNET SETTINGS ------ //
// ------------------------------- //
byte fixedIP[] = { 192, 168, 132, 150 };                   // Use a fixed IP to avoid conflict.
byte resIP[] = { 0, 0, 0, 0 };                             // IP addr container to compare if the ETH conn was successfully established or not?
byte broadcast[] = {192, 168, 132, 255};                   // if we want our system to broadcast feedback upon receiving DMX artnet universe/s
byte querryMAC[] = { 0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D }; // Dummy random MAC addr used for retreiving Teensy 4.1's actual MAC addr
byte teensyMAC[6] = {};                                    // Array to hold the actual MACaddr of Teensy 4.1 (To be used for starting Ethernet Interface later)


void assignMAC(byte *_mac) {
  for (uint8_t by = 0; by < 2; by++) _mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++) _mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
#ifdef DEBUG
  Serial.printf("byte teensyMAC[] = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x };\n", _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
#endif
  for (int i = 0; i < 6; i++) {
    teensyMAC[i] = _mac[i];
  }
}

// Utility to convert the "IPAddress" into a usable type (String),
// for example, you know, may be you want to display it somewhere someday.
//String StringIPAddr(IPAddress address) {
//  return String(address[0]) + "." + String(address[1]) + "." + String(address[2]) + "." + String(address[3]);
//}
//byte *getIPAddr(IPAddress address) {
//  byte IP_ARR[4] = { 0, 0, 0, 0 };
//  for (int i = 0; i < sizeof(IP_ARR); i++) {
//    IP_ARR[i] = address[i];
//  }
//  return IP_ARR;
//}





void setup() {
#ifdef DEBUG
  while (!Serial) {
    ; // wait for serial port to connect
  }
#endif

  // Initiates on board LEDs that are used by our logic to show some status like if "network setup was successful or not..." etc.
  initDebugLeds();

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

  //[TBD] Set bright ness may be ...

  clearLEDs();

  // On init, test WS2811 LEDs
  initLEDTest();

  //  Get and asisgn new found MAC addr of Teensy4.1
  logln("Getting new mac addr...");
  assignMAC(querryMAC);
  logln("Trying to begin ARTNET with Fixed IP and the above MAC addr...");
  delay(1000);

  //  Begin art-net with the new MAC addr and the fixed IP
  artnet.begin(teensyMAC, fixedIP);
  delay(2000);

  log("CURR IP ADDR: ");
  logln(Ethernet.localIP());
  log("ETH LINK STATUS: ");
  logln(Ethernet.linkStatus());


  /* if the fixed IP was assigned successfully, proceed; or else notify and block*/
  //  Since Artnet.begin(mac, ip) actually calls Ethernet.begin(mac, ip) underneath, we can check if our uC got the intended fixed IP address.
  //  If so, proceed.
  //  Or else, notify and block.
  IPAddress currIP = Ethernet.localIP();

  if (currIP[0] != 0 && currIP[1] != 0 && currIP[2] != 0 && currIP[3] != 0 && Ethernet.linkStatus() == 1) {
    logln("ARTNET INIT: ok");
    digitalWrite(NET_INIT_SUCCESS_LED_PIN, HIGH);
    digitalWrite(NET_INIT_FAIL_LED_PIN, LOW);
  } else {
    logln("ARTNET STATUS: fail");
    digitalWrite(NET_INIT_SUCCESS_LED_PIN, LOW);
    digitalWrite(NET_INIT_FAIL_LED_PIN, HIGH);

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
  //  artnet.setArtSyncCallback(onSync);
}

//boolean print_polls = false;
void loop() {
  artnet.read();
}
