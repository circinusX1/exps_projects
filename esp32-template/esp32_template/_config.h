#pragma once

#define   SERIAL_BOS    115200
#define   RELAY         4

#define HAS_I2C         1
#define I2C_SDA         4
#define I2C_SCL         5

#define WITH_GRAPH      1
#define WITH_SHT21      1
#define WITH_NTP        1
#define DEBUG           1







#if DEBUG
#   define TRACE()         Serial.print(__FUNCTION__); Serial.println(__LINE__);
#else
#   define TRACE()
#endif
