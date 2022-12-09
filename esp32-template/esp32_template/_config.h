#pragma once

#define   SERIAL_BOS    115200
#define   RELAY         14

#define HAS_I2C         0
#define I2C_SDA         12
#define I2C_SCL         13

#define WITH_GRAPH      0
#define WITH_SHT21      0
#define WITH_ATH10      0
//0x38
#define WITH_NTP        0
#define DEBUG           0


#if DEBUG
#   define TRACE()         Serial.print(__FUNCTION__); Serial.println(__LINE__);
#else
#   define TRACE()
#endif
