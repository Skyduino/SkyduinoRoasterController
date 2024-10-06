[!["Buy Me A Coffee"](https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png)](https://www.buymeacoffee.com/adminiuga)
# Skyduino Roaster

## What is this?

This is a "pre-alpha" firwmare sources for Skyduino Roaster, a [custom Arduino board](https://github.com/Skyduino/Skyduino-Hardware/) for Skywalker/ITOP MTCR(SkItop) roaster.

⚠️WARNING 👉 DO NOT EVEN CONSIDER USING THIS!!! It's ~~likely~~ going to set your house on Fire⚠️


## Why is this even here?
This project was inspired by the [original work](https://github.com/jmoore52/SkywalkerRoaster) of Josh Moore, who reversed engineered the Skywalker roaster protocol and built a sketch for connecting the roaster and [Artisan](https://artisan-scope.org/) allowing either full control of the roaster or just the data logging.
Also, I wanted to learn something new and building Arduino boards, with a display and a TC interface required too much soldering, so there always was an idea in the back of my mind of building a custom Arduino board which had everything required for this roaster onboard.


# Installation

## Obtain the firmware
Download the firmware from [Skyduino Releases](https://github.com/Skyduino/SkyduinoRoasterController/releases) (no published releases yet, but check again in the future),
build firmware yourself (easiest way is to use the [PlatformIO IDE](https://platformio.org/platformio-ide)
in [VSCode](https://code.visualstudio.com/download)) or as the last resort, try finding a latest
artifact in [Github Actions](https://github.com/Skyduino/SkyduinoRoasterController/actions/workflows/Platformio-ci.yaml) run history.

### Stepper drum motor firmware
The "Firmware Stepper" zip file contains the modified firmware to control the drum
using a stepper motor. This does require a separate stepper motor driver,
controlled through **J13 Disp** header, using the following pins:

| Pin Name | Function | Description |
| --- | --- | --- |
| SCL | STEP | Step signal to stepper driver |
| SDA | nEN | Enable signal to driver, low level is active |

The firmware was tested with TMC2209 driver board. You may need to hardwire the direction pin, as this is not controlled at the moment.


## DFU Mode

The board is based on ST32F412RET6 microcontroller, therefore the easiest way to install or
upgrade the firmware is to use the DFU mode. To enter the DFU mode:
- PowerOff the roaster and unplug the USB-C connection to computer
- hold the boot button
- plug in the USB-C cable to computer and the MCU should be in DFU mode

To flash the firmware, use the `dfu-util` and run `dfu-util -a 0 -s 0x8000000 -D .\firmware.bin`
The `dfu-util` is packaged for the most linux distros.

On Windows:
1. Download [dfu-util][dfu-util-win]
2. Download [Zadig][zadig] USB driver installer for Windows
3. Enter the DFU mode on the board
4. Install `WinUSB` driver for `STM32 BOOTLOADER` using [Zadig][zadig]
5. Flash the firmware: `dfu-util -a 0 -s 0x8000000 -D .\firmware.bin`

There is also [online dfu-util](https://devanlai.github.io/webdfu/dfu-util/) Which works only on Chrome, but this was not tested yet. Can anyone test and let me know?

## Firmware Update
When a new release becomes available, either follow [DFU Mode](#dfu-mode) or you can try switching to DFU mode with a serial command,
either manually, or by running an expect script, e.g. `Scripts/skyduino_dfu.sh /dev/ttyUSB0 115200`
If you cannot run the script, the other option is to switch manually, by connecting with a serial
terminal to the port, and issuing two commands in quick succession. The first command is just `DFU`
which starts DFU mode switching negotiation. In response, the roaster will reply with a number,
e.g. `42641`. For the second command, enter `DFU;` followed by number from the previous command
in our example, the full command is going to be `DFU;42641` There's a ttimer 5 second running in
the background, from the time the 1st dfu commad was executed and the 2nd dfu command was submitted,
if the second command arrives too late, then you have to start from the beginning, by sending an empty `DFU` command.



# Artisan settings
The artisan settings are backward compatible (at least for the moment) with the original Josh's
firmware, but the DRUM speed is now adjustable. Sample config is available in [Settings/ArtisanSettings](./Settings/ArtisanSettings/)

If you are using Linux, then you can also add an udev rule, so the serial port is always available as `/dev/ttyUSB.artisan` The udev rule file is in [Settings/udev/90-arduino-stm32-acm.rules](./Settings/udev/90-arduino-stm32-acm.rules) Copy this file into `/etc/udev/rules.d` then restart the udev daemon with `sudo udevadm control --reload` and replug the board.


# Internals
## Temperature channels
Firmware support TC4 `CHAN` command which allows remapping of the logical to physical channels. Artisan expects **ET** on the logical channel #1 and **BT** on the logical channel #2. On the controller, the physical channel #1 is the Thermocouple Probe and physical channel #2 for the NTC resistive temperature probe.
Thus, `CHAN;2100` configures roaster to use the thermocouple for the **BT** and NTC for the **ET** (just to log the other probe?).
Alternatively, you could use `CHAN;1200` command to use NTC for **BT** and the thermocouple for **ET**. Or, if you don't use the thermoucouple at all, use `CHAN;1000` to disable ET & thermocouple completely.

## Safety
The firmware monitors both physical channels for the max temperature, defined as `MAX_SAFE_TEMP_C` macro in [platformio.ini](https://github.com/Skyduino/SkyduinoRoasterController/blob/4a706247c2a8c93f3a51e89be9654132c200d2fa/platformio.ini#L20)
The safety monitor component requires at least one working temperature channel.
If none of the temperature channels are working (both the NTC & thermocouple are disconnected) then the firmware 
switches into failsafe mode. In other words, if you are testing the hardware/firmware, have either NTC or thermocouple connected, otherwise 
the firmware is going to ignore the control commands, due to failsafe.




[Reference Table]: #
[dfu-util-win]: http://dfu-util.sourceforge.net/releases/dfu-util-0.8-binaries/win32-mingw32/dfu-util-static.exe "dfu-util for Windows"
[zadig]: https://zadig.akeo.ie/ "Zadig USB Driver installer"