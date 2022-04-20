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

  Important notes:
  ----------------
  1. Needed to add manually Adafruit Busio lib from pio's library registry, to this project.
  2. Needed to add manually Adafruit GFX lib from pio's library registry, to this project.
  3. Needed to add manually Adafruit SSD1306 pio's library registry, to this project.
  4. In platformio.ini, in "lib_deps" section, add https://github.com/vjmuzik/NativeEthernet . (although is baked in Teensy System, but it might not be the latest)
  5. In platformio.ini, in "lib_deps" section, add https://github.com/natcl/Artnet . (May be available in pio lib registry, but it's not latest [as of Mar 20222])

  PIO specific instructions: (for 1st time)
  ------------------------------------------
  It will highlight in red complaining it can't find the follwoing libraries in path, in some header files:
    #include <Arduino.h>
    #include <SPI.h>
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>

  1. In vscode, in the appropriate header files, a small light bulb will appear, asking "Fix the quickway", do that (for both the libraries)
  2. Come back to main.cpp and compile, the errors would be gone.

  Future Note:
  -----------
    https://forum.pjrc.com/threads/67780-Teensy-4-1-Native-Ethernet-max-SocketSize-lower-than-set?highlight=native+ethernet
    Also, I don't know when it was added), with NativeEthernet, you can use Ethernet.begin(mac, 0, 0) which does not block (even without ethernet cable)
    and takes around 5 millis to run (on T4.1). After that you can use: Ethernet.linkStatus() to know the status of the link. If no cable was inserted
    when "begin" was run, and you insert it later, a DHCP address will get assigned in non-blocking.
*/

#include <Arduino.h>

#include "config.h"
#include "debug_desc.h"
#include "oled_m.h"
#include "neopixel_strips.h"
#include "network.h"
#include "artnet_methods.h"

void initDebugLeds()
{
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
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
  startArtnetMethods();
}

void loop()
{
  readArtnet();
}