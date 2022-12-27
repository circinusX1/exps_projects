#pragma once

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Ticker.h>
#include <NTPClient.h>


#define USER_SIG        0x13
#define RAMSIG          0x21
#define BOGUS_VAL       999999.99   // let's hope none of sensors hit this

#define MAX_SENS            2   // max connected I2C sensors
#define S_POINTS              2   // max data points per sensor


#define MKVER(a,b,c)    a<<16|b<<8|c

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

#define REBOOT()    do{ static void (*_JMPZERO)(void)=nullptr; cli(); (_JMPZERO)(); } while(0);
inline void __nill(){}


typedef enum ETYP{
    eNA=0,
    eTIME,
    eRELAY,
    eTEMP,
    eHUM,
    ePRESS,
    eREMOTE,
};


//////////////////////////////////////////////////////////////////////////////////////////
class sens_data_t;
class unidata_t
{
public:
    static void    release(const unidata_t* p){delete p;}
    static unidata_t* get(ETYP e, float d){return new unidata_t(e,d);}
    void           add(ETYP e, float d){
        unidata_t* next=nullptr;
        if(next==nullptr){
            next=new unidata_t(e,d);
            return;
        }
        next->add(e,d);
   }
private:
    unidata_t(ETYP e, float d):type(e),data(d){}
    unidata_t(ETYP e):type(e){}
    ~unidata_t(){delete next;}

public:
    ETYP  type;
    float data;
    unidata_t* next=0;
};



#define FORCE_INLINE __attribute__((always_inline)) inline
#define COUNT_OF(a_) sizeof(a_)/sizeof(a_[0])
