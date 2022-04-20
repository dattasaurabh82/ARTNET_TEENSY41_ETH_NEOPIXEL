/**
 * @brief
 *
 */

#include <SPI.h>
// # [TBD] This will compile for Teensy 4.1 and Tennsy 4.0 ([TBD]: Differentiate fromTeensy 4.0 as it doesn't have Native Ethernet)
//#if defined( __IMXRT1062__)
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
// IPAddress remoteIP = {192, 168, 132, 255};
//#endif
#include <Artnet.h>

byte querryMAC[] = {0xE5, 0x2A, 0xFC, 0x41, 0x13, 0x2D}; // Dummy random MAC addr used for retreiving Teensy 4.1's actual MAC addr
byte teensyMAC[6] = {};                                  // Array to hold the actual MACaddr of Teensy 4.1 (To be used for starting Ethernet Interface later)

Artnet artnet;
const int startUniverse = 0;                           // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.
const int numberOfChannels = numLeds * channelsPerLed; // Total number of channels you want to receive over DMX

const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0); // Check if we got all universes...
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

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