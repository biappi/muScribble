# µScribble

An hardware USB [Scribble Strip](https://www.sweetwater.com/insync/scribble-strip/)
 that supports the Mackie Control Protocol.

<img
  src="https://github.com/biappi/muScribble/blob/master/hardware/images/mu-scribble.gif?raw=true"
  alt="µScribble in action"
  width="640" height="360"
/>


## Introduction

µScribble is an USB Midi gadget designed to be a companion to inexpensive faderboxes
without displays. It should be compatible with any DAW that supports control surfaces.
I've personally used it with [Apple Logic Pro](https://www.apple.com/logic-pro/) and
[Ableton Live](https://www.ableton.com/en/live/).

The hardware itself is very simple an d based around an
[STM32F4](https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html)
evaluation board, sometimes known as "blackpill", while the displays are the SPI variant
of the ubiquitous [SSD1306 OLED screens](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf).

The software is implemented using the [unicore-mx](https://github.com/insane-adding-machines/unicore-mx)
fork of [libopencm3](https://github.com/libopencm3/libopencm3) as it has a slighlty
better support for the stm32 target platform. It should be relatively easy to port
the application code to any microcontroller of choice.


## Hardware design files

The hardware is designed using the [KiCad EDA](https://kicad-pcb.org)

* [KiCad project directory](hardware/kicad)
* [Schematic diagram PDF](hardware/images/schema.pdf)

  <img
    src="https://github.com/biappi/muScribble/blob/master/hardware/images/schema-thumb.png?raw=true"
    alt="Schematic diagram thumbnail"
    width="640"
    height="453"
  />

* [Gerber fabrication files directory](hardware/gerbers)

  <img
    src="https://github.com/biappi/muScribble/blob/master/hardware/images/pcb-thumb.png?raw=true"
    alt="PCB diagram thumbnail"
    width="640"
    height="80"
  />
  
  
## Software

The firmware depends on the `arm-none-eabi-gcc` toolchain, and to compile it
it's sufficient to invoke

```
make
```

As a convenience, if you have the [STLink](https://github.com/stlink-org/stlink/)
tools installed, you can flash the firmware to the device using the `st-flash` command
with

```
make flash
```
