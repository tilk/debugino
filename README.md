# Debugino

An AVR ATmega development board with GDB debugging support via DebugWIRE.
Intended for educational purposes.

## Introduction

The Debugino board contains two microcontrollers:

 * an AVR ATmega328P (the same as in the popular Arduino Nano boards),
 * an STM32F103RBT6, which acts as the programmer, debugger and USB-to-UART.

The USB interface presents two CDC ACM virtual serial ports.
The first one allows to communicate with the UART on the ATmega.
The second implements two protocols:

* STK500v2, which allows programming the board using `avrdude` or other popular programming tools,
* GDB Remote Serial Protocol, which allows debugging using `avr-gdb`.

Programming and debugging the ATmega is performed using the DebugWIRE protocol.
Therefore no special bootloader or other software is needed on the ATmega.

To reduce component count, the board does not contain a separate crystal oscillator for the ATmega.
The clock for the ATmega is generated using a timer on the STM32, set to 18 MHz.

## Preparing a board

The STM32 is programmed via a separate ISP connector which exposes the SWDIO and SWCLK signals.

The ATmega does not require a bootloader, but the fusebits need to be changed from factory settings to enable DebugWIRE and configure the system clock.
The preferred fusebits values are:

* LFUSE 0xE0,
* HFUSE 0x9F,
* EFUSE 0xFF.

There is no ISP connector for the ATmega on the board, but the required signals are available on the pins labelled B3 (MOSI), B4 (MISO), B5 (SCK) and DWI (RESET).
