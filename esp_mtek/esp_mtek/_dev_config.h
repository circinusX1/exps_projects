#ifndef _DEV_CONFIG_H
#define _DEV_CONFIG_H

#include "_utils.h"
#include "_boards.h"

/////////////////////////////////////////////////////////////////////////////////////////
#define VERSION         MKVER(1,0,0);

/////////////////////////////////////////////////////////////////////////////////////////
#define MY_BOARD        WSP_8266_GEN

#define   MAX_PROGRAM_SZ  480000
#define   SERIAL_BPS      115200

/////////////////////////////////////////////////////////////////////////////////////////
#if MY_BOARD==WSP_8266_GEN
#   define   MAX_PROGRAM_SZ  480000
#   define I2C_SDA         4
#   define I2C_SCL         5
#   define LED             2
#   define RELAY           14   // D5
#   define BUTTON          0
#   define WITH_GRAPH      1
#   define WITH_NTP        1
#elif MY_BOARD==ESP_WITH_RELAY
#   define   MAX_PROGRAM_SZ  480000
#   define I2C_SDA         12
#   define I2C_SCL         13   // on a side
#   define LED             2
#   define RELAY           4
#   define BUTTON          0
#   define WITH_GRAPH      1
#   define WITH_NTP        1
#elif MY_BOARD==GLOBE_SMART_PLUG || MY_BOARD==EZVIZ_T30
    #define   MAX_PROGRAM_SZ  340000
#   define I2C_SDA          0
#   define I2C_SCL          0
#   define LED              4
#   define RELAY            12
#   define BUTTON           14
#   define WITH_GRAPH       0
#   define WITH_NTP         1
#elif MY_BOARD==EZVIZ_T30

#endif




#endif // _DEV_CONFIG_H
