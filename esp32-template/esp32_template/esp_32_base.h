#ifndef AP_WIFI_OTA
#define AP_WIFI_OTA

#include "_config.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include "Ticker.h"
#if WITH_NTP
#include "NTPClient.h"
#endif



#define REBOOT()    do{ static void (*_JMPZERO)(void)=nullptr; cli(); (_JMPZERO)(); } while(0);

#define AP_BLINK     50
#define ON_BLINK     300
#define OFF_BLINK    800



struct RAMM{
    int relay_state;
    bool fail;
    int  sig;
};

extern RAMM __Ramm; // __attribute__ ((section (".noinit")));

#define SAMPLES_MAX  2880
#define SIG  0x58


class esp_32_base_c
{
public:
    esp_32_base_c();
    virtual ~esp_32_base_c();

    bool    setup();
    bool    loop();
    virtual void  user_loop(unsigned int) = 0;
    virtual void  user_begin() = 0;
    virtual void  page_request(ESP8266WebServer* srv,
                               String& page)=0;

   template <class T>void save_data(const  T& t){
      EEPROM.begin(512);
      delay(256);
      EEPROM.put(sizeof(_eprom)+1, t);
      EEPROM.commit();
      EEPROM.end();
    }
    template <class T>void load_data(T& t){
        EEPROM.begin(512);
        delay(256);
        EEPROM.get(sizeof(_eprom)+1, t);
        EEPROM.end();
        delay(100);
    }

public:
    static void tick_tack();
    static void handleRoot();
    static void handleWifi();
    static void handleOta();
    static void handleTime();
    static void handleWifiSave();
    static void handleNotFound();
    static void handleI2c();
    static void otaUpdate();
    static void IRAM_ATTR delayMicroseconds2(uint32_t us);

protected:
    void _loadCredentials();
    void _saveCredentials();
    void _connectWifi();
    static boolean _isIp(String str);
    static String _toStringIp(IPAddress ip) ;
    const __FlashStringHelper * _start_html(bool content=true);
    const String _end_html();
    boolean _capturePage();

protected:
   // DNSServer*           _dns_srv = nullptr;
    ESP8266WebServer*    _esp_srv = nullptr; //(80);
    IPAddress*           _ip_addr = nullptr;    //(10, 5, 5, 1);
    IPAddress*           _net_mask = nullptr;   //(255, 255, 0, 0);
    boolean              _try_connect = false;
    boolean              _bsta = false;
    struct {
    byte                   sig;
    char                 _cur_ssid_name[32]; // = "marius";
    char                 _cur_ssid_pswd[32]; // = "myssidpass";
    char                 _ipstatic[32];
    char                 _offset[16];
    } _eprom;
    int                 _timezone = 0;
    unsigned long       _last_conn = 0;
    unsigned int        _wlan_status = WL_IDLE_STATUS;
    bool                _otaing = false;
    unsigned int        _loop = 0;
    uint32_t            _blink_rate = OFF_BLINK;
    uint32_t            _blinktime = 0;
    byte                _toggle = 0;
    byte                _ip[4];
    int                 _seconds = 0;
    int                 _graphtime = 0;
    unsigned long       _ntptime = 0;
    Ticker              _timer;
    bool                _otaenabled = false;
#if WITH_NTP
    WiFiUDP     _ntpUDP;
    NTPClient*  _timeClient;
#endif

};

extern EEPROMClass EEPROM;

#endif
