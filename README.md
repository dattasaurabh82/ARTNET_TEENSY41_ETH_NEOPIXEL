# ARTNET TEENSY41 ETH NEOPIXEL

![build and release](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/actions/workflows/main.yml/badge.svg)

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

3. GERBER: [TeensyEthernetNeoPixel v21 v444_2022-04-11.zip](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/blob/main/GERBERS/TeensyEthernetNeoPixel%20v21%20v444_2022-04-11.zip)

![TeensyEthernetNeoPixel v59](https://user-images.githubusercontent.com/4619862/158967454-083e91c5-0bb7-464e-9c60-529dc53f1da3.png)
![TeensyArtnetController](https://user-images.githubusercontent.com/4619862/158972039-5d99bbad-d773-4283-83ec-335e74642918.jpeg)

## Development Environment Setup

### 1. __Development environment:__ [Teensyduino 1.56](https://www.pjrc.com/teensy/td_download.html) (as of Mar 2022)

---
__Library Installation notes:__

1. Remove the compiled Artnet library from the Teensyduino IDE App. On my mac, I did it like this:

```shell
mv /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/Artnet /Applications/Teensyduino.app/Contents/Java/hardware/teensy/avr/libraries/.Artnet_old
```

2. Install the latest Artnet library in the Arduino's Library dirtectory (as of Mar 2022) (do not install it using Arduino's Library Manager): `git clone https://github.com/natcl/Artnet.git`
3. Remove the NativeEthernet library (if you have installed it in Arduino library directory previously) as it will be there in the Teensyduino IDE app.
4. Install the Adafruit Neopixel library using Arduino's LIbrary Manager (or Teensyduino's Library Manager)  

### 2. __arduino-cli setup:__

For headless compiling and uploading

Install or upgrade the latest [arduino-cli](https://github.com/arduino/arduino-cli)

__Note:__ Install libraries (listed above) using cli, if you are using in an headless environment where this was never compiled before.
Since, in my test system (also my development system), the listed libraries were already installed (from the above steps), I didn;t need to use it using `arduino-cli`

```shell
arduino-cli config add board_manager.additional_urls https://www.pjrc.com/teensy/td_156/package_teensy_index.json
arduino-cli config dump --verbose
arduino-cli core search teensy
arduino-cli core install teensy:avr
arduino-cli board details -b teensy:avr:teensy41

#COMPILE
cd ARTNET_TEENSY41_ETH_NEOPIXEL
arduino-cli compile -b teensy:avr:teensy41:usb=serial,speed=600,opt=o2std,keys=en-us --output-dir build/ --clean -v

#UPLOAD
arduino-cli compile -b teensy:avr:teensy41:usb=serial,speed=600,opt=o2std,keys=en-us --output-dir build/ --clean -u -p <port> -v

# ENABLE/DISABLE serial debug:
# We just have to set DEBUG=true/flase for that and we can change that MACRO during our compilation as such:
# build.extra_flags=-DDEBUG=false
# Disable Serial Debug
arduino-cli compile -b teensy:avr:teensy41:usb=serial,speed=600,opt=o2std,keys=en-us --build-property build.extra_flags=-DDEBUG=false --output-dir build/ --clean -u -p <port> -v
#Enable Serial Debug
arduino-cli compile -b teensy:avr:teensy41:usb=serial,speed=600,opt=o2std,keys=en-us --build-property build.extra_flags=-DDEBUG=true --output-dir build/ --clean -u -p <port> -v
```

_As the teensy sub-system comes with some built in libraries baked in, so just make sure the latest ones are installed and usally by the users (but sometimes it can be a good idea to use the baked ones instead as they might be otimized for Teensy platforms). But for this repo, as of Mar 2022, I found that most libraries installed by the user in Arduino's lib dir is preferrable to be used in the firmware then t5he ones that came baked in the Teensyduino._

```shell
Multiple libraries were found for "NativeEthernet.h"
Used: /Users/<user>/Documents/Arduino/libraries/NativeEthernet
Not used: /Users/<user>/Library/Arduino15/packages/teensy/hardware/avr/1.56.1/libraries/NativeEthernet
Multiple libraries were found for "Adafruit_SSD1306.h"
Used: /Users/<user>/Documents/Arduino/libraries/Adafruit_SSD1306
Not used: /Users/<user>/Documents/Arduino/libraries/Adafruit_SSD1306_Wemos_Mini_OLED
Multiple libraries were found for "Adafruit_I2CDevice.h"
Used: /Users/<user>/Documents/Arduino/libraries/Adafruit_BusIO
Not used: /Users/<user>/Documents/Arduino/libraries/arduino_789302
Multiple libraries were found for "Adafruit_NeoPixel.h"
Used: /Users/<user>/Documents/Arduino/libraries/Adafruit_NeoPixel
Not used: /Users/<user>/Library/Arduino15/packages/teensy/hardware/avr/1.56.1/libraries/Adafruit_NeoPixel
Multiple libraries were found for "Artnet.h"
Used: /Users/<user>/Documents/Arduino/libraries/Artnet
Not used: /Users/<user>/Library/Arduino15/packages/teensy/hardware/avr/1.56.1/libraries/Artnet
```

### 3. __Development environment:__ [platformio](https://platformio.org/) (in VSCode)

---
PIO specific instructions: (from scratch)

1. Add manually __Adafruit Busio lib__ from pio's library registry, to this project.
2. Add manually __Adafruit GFX lib__ from pio's library registry, to this project.
3. Add manually __Adafruit SSD1306__ pio's library registry, to this project.
4. These steps will update the `platformio.ini` file.
5. Then, in `main.cpp` try importing the following libs:

    ```c++
    #include <Arduino.h>
    #include <SPI.h>
    #include <Wire.h>
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>
    ```

6. Some of them will get highlight in red, complaining that those couldn't be found in the library's PATHs.
7. In vscode, here a small light bulb will appear, asking "Fix it in a quick way"; do that (for for the highlighted libraries)
8. Come back to `main.cpp` and compile, the ERRORS would be gone.
9. Then we need to add 2 more libraries in the `platformio.ini`.
10. In `lib_deps:` section, add <https://github.com/vjmuzik/NativeEthernet> (although is baked in Teensy System, but it might not be the latest).
11. In `platformio.ini`, in `lib_deps:` section, add <https://github.com/natcl/Artnet>, (__Note:__ **In pio lib registry, there is the Artnet library, but it's not latest [as of Mar 20222])

---

#### Minimal [platformio.ini](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/blob/main/platformio_alternative/teensy41_pio_artnet_demo/platformio.ini)

```yaml
[env:teensy41]
platform = teensy
board = teensy41
framework = arduino
board_build.mcu = imxrt1062
board_build.f_cpu = 600000000L ; max freq available on T4.1
monitor_speed = 115200
upload_protocol = teensy-cli ; teensy-gui is default also "jlink" is available
lib_deps = 
    SPI
    Wire
    https://github.com/vjmuzik/NativeEthernet
    https://github.com/natcl/Artnet
    adafruit/Adafruit NeoPixel@^1.10.4
    adafruit/Adafruit BusIO@^1.11.3
    adafruit/Adafruit GFX Library@^1.10.14
    adafruit/Adafruit SSD1306@^2.5.1
```

---

__License:__  [MIT](https://github.com/dattasaurabh82/ARTNET_TEENSY41_ETH_NEOPIXEL/blob/main/LICENSE)
