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

  PIO specific instractions: (from scratch)
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

#define DEBUG

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

#include <SPI.h>
#include <Wire.h>

#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>

// ------------------------------- //
// ------ ETHERNET SETTINGS ------ //
// ------------------------------- //
byte fixedIP[] = {192, 168, 132, 150};                   // Use a fixed IP to avoid conflict.
byte resIP[] = {0, 0, 0, 0};                             // IP addr container to compare if the ETH conn was successfully established or not?
byte querryMAC[] = {0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D}; // Dummy random MAC addr used for retreiving Teensy 4.1's actual MAC addr
// uint8_t qm[] = {0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D};
byte teensyMAC[6] = {};

void assignMAC(byte *_mac)
{
  for (uint8_t by = 0; by < 2; by++)
    _mac[by] = (HW_OCOTP_MAC1 >> ((1 - by) * 8)) & 0xFF;
  for (uint8_t by = 0; by < 4; by++)
    _mac[by + 2] = (HW_OCOTP_MAC0 >> ((3 - by) * 8)) & 0xFF;
  Serial.printf("byte teensyMAC[] = { 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x };\n", _mac[0], _mac[1], _mac[2], _mac[3], _mac[4], _mac[5]);
  for (int i = 0; i < 6; i++)
  {
    teensyMAC[i] = _mac[i];
  }
}

// String DisplayIPAddress(IPAddress address) {
//   return String(address[0]) + "." + String(address[1]) + "." + String(address[2]) + "." + String(address[3]);
// }

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  logln("------------ ETHERNET INTERFACE INIT ---------------");
  logln("Getting new mac addr...");
  delay(1000);
  //  get and asisgn new found mac
  assignMAC(querryMAC);
  delay(1000);
  logln("\nTrying DHCP to get an IP addr assigned to th e uC ...");
  delay(1000);

  if (Ethernet.begin(teensyMAC) == 0)
  {
    logln("[x] Failed to configure Ethernet using DHCP ");
    logln("\nTrying Fixed IP...");
    delay(1000);
    Ethernet.begin(teensyMAC, fixedIP);
    delay(1000);
    log("\nFIXED IP: ");
    logln(Ethernet.localIP());
  }
  else
  {
    log("\nDHCP IP: ");
    logln(Ethernet.localIP());
  }
  log("ETH LINK STATUS: ");
  logln(Ethernet.linkStatus());

  IPAddress currIP = Ethernet.localIP();

  if (currIP[0] != 0 && currIP[1] != 0 && currIP[2] != 0 && currIP[3] != 0 && Ethernet.linkStatus() == 1)
  {
    logln("\nETHERNET INITIATED: OK! \n");
  }
  else
  {
    // Draw IP addr on the oled display
    logln("\nETHERNET INITIATED: FAILED! \n");
  }
}

void loop()
{
}