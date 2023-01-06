

#pragma once

#include "_my_config.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <Ticker.h>
#if WITH_NTP
#   include <NTPClient.h>
#endif

/////////////////////////////////////////////////////////////////////////////////////////
#define USER_SIG        0x13
#define RAMSIG          0x21
#define BOGUS_VAL       999999.99
#define MKVER(a,b,c)    a<<16|b<<8|c

//////////////////////////////////////////////////////////////////////////////////////////
#if DEBUG
#   define TRACE()         Serial.print(__FUNCTION__); Serial.println(__LINE__);
#else
#   define TRACE()
#endif

//////////////////////////////////////////////////////////////////////////////////////////
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

/////////////////////////////////////////////////////////////////////////////////////////
#define REBOOT()    ESP.restart(); //do{ static void (*_JMPZERO)(void)=nullptr; cli(); (_JMPZERO)(); } while(0);
inline void __nill(){}

/////////////////////////////////////////////////////////////////////////////////////////
enum ETYP{
    eNA=0,
    eTIME,
    eRELAY,
    eTEMP,
    eHUM,
    ePRESS,
};


//////////////////////////////////////////////////////////////////////////////////////////
class sens_data_t;
class sensdata_t
{
public:
    static void    release(const sensdata_t* p){delete p;}
    template <typename T>
    static sensdata_t* get(ETYP e, T d){return new sensdata_t(e,d);}
    void add(ETYP e, float d){
        sensdata_t* next=nullptr;
        if(next==nullptr){
            next=new sensdata_t(e,d);
            return;
        }
        next->add(e,d);
   }
private:
    sensdata_t(ETYP e, float d):type(e){u.f=d;}
    sensdata_t(ETYP e, bool d):type(e){u.uc=d;}
    sensdata_t(ETYP e, int d):type(e){u.i=d;}
    sensdata_t(ETYP e):type(e){}
    ~sensdata_t(){delete next;}

public:
    ETYP  type;
    union{ float f; size_t z; uint8_t uc; int16_t ss; uint16_t us; int i;}u;
    sensdata_t* next=0;
};


#define FORCE_INLINE __attribute__((always_inline)) inline

#define COUNT_OF(a_) sizeof(a_)/sizeof(a_[0])

#define BTIME __TIME__

#define _S(xx)  String(xx)
