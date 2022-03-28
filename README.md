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

1. Schematic: [Schematuc.pdf](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/blob/main/Schematic.pdf)
![Screenshot 2022-03-18 at 6 06 32 PM](https://user-images.githubusercontent.com/4619862/158983284-2409939a-90e1-4d3d-ba66-2f355a3bd0c1.png)

2. PCB deisgn:

3. GERBER: [TeensyEthernetNeoPixel v21 v317_2022-03-04.zip](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/blob/main/GERBER/TeensyEthernetNeoPixel%20v21%20v317_2022-03-04.zip)

![TeensyEthernetNeoPixel v59](https://user-images.githubusercontent.com/4619862/158967454-083e91c5-0bb7-464e-9c60-529dc53f1da3.png)
![TeensyArtnetController](https://user-images.githubusercontent.com/4619862/158972039-5d99bbad-d773-4283-83ec-335e74642918.jpeg)

---

## Development Environment Setup

### __Development environment:__ [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html) (as of Mar 2022)

1. __Library Installation:__

    1. Remove the compiled Artnet library from the Teensyduino IDE App. On my mac, I did it like this:

    ```shell
    mv /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/Artnet /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/.Artnet_old
    ```

    2. Install the latest Artnet library in the Arduino's Library dirtectory (as of Mar 2022) (do not install it using Arduino's Library Manager): `git clone https://github.com/natcl/Artnet.git`
    3. Remove the NativeEthernet library (if you have installed it in Arduino library directory previously) as it will be there in the Teensyduino IDE app.
    4. Install the Adafruit Neopixel library using Arduino's LIbrary Manager (or Teensyduino's Library Manager)  

### __Development environment:__ platformio (in VSCode)

PIO specific instractions: (from scratch)

    1. Add manually Adafruit Busio lib from pio's library registry, to this project
    2. Add manually Adafruit GFX lib from pio's library registry, to this project
    3. Add manually Adafruit SSD1306 pio's library registry, to this project
    4. In main.cpp try importing the following libs:

    ```c++
    #include <Arduino.h>
    #include <SPI.h>
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
    ```

    5. It will highlight in red complaining it couldn't find the libariues in path.
    6. In vscode, here a small light bulb will appear, asking "Fix the quickway", do that (for both the libraries)
    7. Come back to main.cpp and compile, the errors would be gone.
    8. In platformio.ini, in "lib_deps" section, add https://github.com/vjmuzik/NativeEthernet (although is baked in Teensy System, but it might not be the latest)
    9. In platformio.ini, in "lib_deps" section, add https://github.com/natcl/Artnet, (May in pio lib registry, there is the Artnet library, but it's not latest [as of Mar 20222])

__License:__  (MIT)[]
