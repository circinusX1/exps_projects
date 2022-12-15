#pragma once

#define   SERIAL_BOS    115200
#define   RELAY         14
#define   LED           2

#define HAS_I2C         1
//#define I2C_SDA         12
//#define I2C_SCL         13

#define I2C_SDA         4
#define I2C_SCL         5

#define WITH_GRAPH      1
#define WITH_SHT21      0
#define WITH_ATH10      1
//0x38
#define WITH_NTP        1
#define DEBUG           0


#if DEBUG
#   define TRACE()         Serial.print(__FUNCTION__); Serial.println(__LINE__);
#else
#   define TRACE()
#endif
