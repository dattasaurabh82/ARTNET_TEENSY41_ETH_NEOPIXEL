# ARTNET TEENSY41 ETH NEOPIXEL

__Platform:__ Teensy 4.1 @ 400MHz with Native Ethernet

An ARTNET based multi single strip neopixel (WS2811) system; running on Teensy 4.1 (using NativeEthernet and Artnet) where it receives DMX universe 0 and sets them parallel to all 4 strips

__Credits:__

1. PaulStoffregen : for the [Teensy platform](https://github.com/PaulStoffregen/cores) and initial Ethernet library.
2. Nathanaël Lécaudé : for the [Artnet library](https://github.com/natcl/Artnet)
3. vjmuzik: For the Native [Ethernet library](https://github.com/vjmuzik/NativeEthernet)
4. Adafruit : for the [Neopixel libray](https://github.com/adafruit/Adafruit_NeoPixel)

---

## Hardware Setup

1. Schematic:

2. PCB deisgn:

3. GERBER:

![TeensyEthernetNeoPixel v59](https://user-images.githubusercontent.com/4619862/158967454-083e91c5-0bb7-464e-9c60-529dc53f1da3.png)
![TeensyArtnetController](https://user-images.githubusercontent.com/4619862/158972039-5d99bbad-d773-4283-83ec-335e74642918.jpeg)

---

## Development Environment Setup

1. __Development environment:__ [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html) (as of Mar 2022)

2. __Library Installation:__

    1. Remove the compiled Artnet library from the Teensyduino IDE App. On my mac, I did it like this:

    ```shell
    mv /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/Artnet /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/.Artnet_old
    ```

    2. Install the latest Artnet library in the Arduino's Library dirtectory (as of Mar 2022) (do not install it using Arduino's Library Manager): `git clone https://github.com/natcl/Artnet.git`
    3. Remove the NativeEthernet library (if you have installed it in Arduino library directory previously) as it will be there in the Teensyduino IDE app.
    4. Install the Adafruit Neopixel library using Arduino's LIbrary Manager (or Teensyduino's Library Manager)  

## Program's Context

---

__License:__  (MIT)[]
