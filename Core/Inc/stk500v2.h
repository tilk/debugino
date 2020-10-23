#ifndef __STK500V2_H
#define __STK500V2_H

//**** ATMEL AVR - A P P L I C A T I O N   N O T E  ************************
//*
//* Title:		AVR068 - STK500 Communication Protocol
//* Filename:		command.h
//* Version:		1.0
//* Last updated:	31.01.2005
//*
//* Support E-mail:	avr@atmel.com
//*
//**************************************************************************

// *****************[ STK message constants ]***************************

#define STK500v2_MESSAGE_START                       0x1B        //= ESC = 27 decimal
#define STK500v2_TOKEN                               0x0E

// *****************[ STK general command constants ]**************************

#define STK500v2_CMD_SIGN_ON                         0x01
#define STK500v2_CMD_SET_PARAMETER                   0x02
#define STK500v2_CMD_GET_PARAMETER                   0x03
#define STK500v2_CMD_SET_DEVICE_PARAMETERS           0x04
#define STK500v2_CMD_OSCCAL                          0x05
#define STK500v2_CMD_LOAD_ADDRESS                    0x06
#define STK500v2_CMD_FIRMWARE_UPGRADE                0x07


// *****************[ STK ISP command constants ]******************************

#define STK500v2_CMD_ENTER_PROGMODE_ISP              0x10
#define STK500v2_CMD_LEAVE_PROGMODE_ISP              0x11
#define STK500v2_CMD_CHIP_ERASE_ISP                  0x12
#define STK500v2_CMD_PROGRAM_FLASH_ISP               0x13
#define STK500v2_CMD_READ_FLASH_ISP                  0x14
#define STK500v2_CMD_PROGRAM_EEPROM_ISP              0x15
#define STK500v2_CMD_READ_EEPROM_ISP                 0x16
#define STK500v2_CMD_PROGRAM_FUSE_ISP                0x17
#define STK500v2_CMD_READ_FUSE_ISP                   0x18
#define STK500v2_CMD_PROGRAM_LOCK_ISP                0x19
#define STK500v2_CMD_READ_LOCK_ISP                   0x1A
#define STK500v2_CMD_READ_SIGNATURE_ISP              0x1B
#define STK500v2_CMD_READ_OSCCAL_ISP                 0x1C
#define STK500v2_CMD_SPI_MULTI                       0x1D

// *****************[ STK PP command constants ]*******************************

#define STK500v2_CMD_ENTER_PROGMODE_PP               0x20
#define STK500v2_CMD_LEAVE_PROGMODE_PP               0x21
#define STK500v2_CMD_CHIP_ERASE_PP                   0x22
#define STK500v2_CMD_PROGRAM_FLASH_PP                0x23
#define STK500v2_CMD_READ_FLASH_PP                   0x24
#define STK500v2_CMD_PROGRAM_EEPROM_PP               0x25
#define STK500v2_CMD_READ_EEPROM_PP                  0x26
#define STK500v2_CMD_PROGRAM_FUSE_PP                 0x27
#define STK500v2_CMD_READ_FUSE_PP                    0x28
#define STK500v2_CMD_PROGRAM_LOCK_PP                 0x29
#define STK500v2_CMD_READ_LOCK_PP                    0x2A
#define STK500v2_CMD_READ_SIGNATURE_PP               0x2B
#define STK500v2_CMD_READ_OSCCAL_PP                  0x2C

#define STK500v2_CMD_SET_CONTROL_STACK               0x2D

// *****************[ STK HVSP command constants ]*****************************

#define STK500v2_CMD_ENTER_PROGMODE_HVSP             0x30
#define STK500v2_CMD_LEAVE_PROGMODE_HVSP             0x31
#define STK500v2_CMD_CHIP_ERASE_HVSP                 0x32
#define STK500v2_CMD_PROGRAM_FLASH_HVSP  `     0x33
#define STK500v2_CMD_READ_FLASH_HVSP                 0x34
#define STK500v2_CMD_PROGRAM_EEPROM_HVSP             0x35
#define STK500v2_CMD_READ_EEPROM_HVSP                0x36
#define STK500v2_CMD_PROGRAM_FUSE_HVSP               0x37
#define STK500v2_CMD_READ_FUSE_HVSP                  0x38
#define STK500v2_CMD_PROGRAM_LOCK_HVSP               0x39
#define STK500v2_CMD_READ_LOCK_HVSP                  0x3A
#define STK500v2_CMD_READ_SIGNATURE_HVSP             0x3B
#define STK500v2_CMD_READ_OSCCAL_HVSP                0x3C

// *****************[ STK status constants ]***************************

// Success
#define STK500v2_STATUS_CMD_OK                       0x00

// Warnings
#define STK500v2_STATUS_CMD_TOUT                     0x80
#define STK500v2_STATUS_RDY_BSY_TOUT                 0x81
#define STK500v2_STATUS_SET_PARAM_MISSING            0x82

// Errors
#define STK500v2_STATUS_CMD_FAILED                   0xC0
#define STK500v2_STATUS_CKSUM_ERROR                  0xC1
#define STK500v2_STATUS_CMD_UNKNOWN                  0xC9

// *****************[ STK parameter constants ]***************************
#define STK500v2_PARAM_BUILD_NUMBER_LOW              0x80
#define STK500v2_PARAM_BUILD_NUMBER_HIGH             0x81
#define STK500v2_PARAM_HW_VER                        0x90
#define STK500v2_PARAM_SW_MAJOR                      0x91
#define STK500v2_PARAM_SW_MINOR                      0x92
#define STK500v2_PARAM_VTARGET                       0x94
#define STK500v2_PARAM_VADJUST                       0x95
#define STK500v2_PARAM_OSC_PSCALE                    0x96
#define STK500v2_PARAM_OSC_CMATCH                    0x97
#define STK500v2_PARAM_SCK_DURATION                  0x98
#define STK500v2_PARAM_TOPCARD_DETECT                0x9A
#define STK500v2_PARAM_STATUS                        0x9C
#define STK500v2_PARAM_DATA                          0x9D
#define STK500v2_PARAM_RESET_POLARITY                0x9E
#define STK500v2_PARAM_CONTROLLER_INIT               0x9F

// *****************[ STK answer constants ]***************************

#define STK500v2_ANSWER_CKSUM_ERROR                  0xB0

#endif

