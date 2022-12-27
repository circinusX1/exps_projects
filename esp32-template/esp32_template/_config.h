#pragma once

#include <EEPROM.h>


#define   SERIAL_BOS    115200
#define   RELAY         4         // 4 the 12-30v relay
#define   LED           2

#define HAS_I2C         0
#define I2C_SDA         12        // 12v relay
#define I2C_SCL         13

//#define I2C_SDA         4       // 12v relay
//#define I2C_SCL         5

#define WITH_GRAPH      1
#define WITH_SHT21      1
#define WITH_ATH10      1
//0x38
#define WITH_NTP        1
#define DEBUG           0


#if WITH_SHT21 || WITH_ATH10
#   undef HAS_I2C
#   define HAS_I2C 1
#endif


#if DEBUG
#   define TRACE()         Serial.print(__FUNCTION__); Serial.println(__LINE__);
#else
#   define TRACE()
#endif


inline void LOG( const char * format, ... )
{
    char lineBuffer[64];
    va_list args;
    va_start( args, format );
    int len = vsnprintf( lineBuffer, sizeof lineBuffer, (char*) format, args );
    Serial.print( lineBuffer );
    Serial.println();
    va_end( args );
}



inline void __nill(){}
