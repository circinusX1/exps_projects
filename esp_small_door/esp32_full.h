#ifndef AP_WIFI_OTA
#define AP_WIFI_OTA

#include "_config.h"
#include "time.h"
  #include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#define REBOOT()    do{ static void (*_JMPZERO)(void)=nullptr; cli(); (_JMPZERO)(); } while(0);

#define AP_BLINK     50
#define ON_BLINK     300
#define OFF_BLINK    1800


struct RAMM{
    int relay_state;
};

extern RAMM __Ramm; // __attribute__ ((section (".noinit")));

#define SAMPLES_MAX  2880
#define SIG  0x60

struct Temps
{
    int   _count;
    float _data[SAMPLES_MAX];
};


class esp32_full
{
public:
    esp32_full(int);
    virtual ~esp32_full();

    bool    setup();
    bool    loop();
    virtual void  user_loop(unsigned int) = 0;
    virtual void  page_request(ESP8266WebServer* srv,
                               String& page)=0;
    int getOnTime()const{return _eprom._ontimemin;}
    int getTz()const{return _eprom._timezone;}
    int getOffTime()const{return _eprom._offtimemin;}
    const char* getNTPSrv()const{return _eprom._ntpsrv;}
    
public:
    static void handleRoot();
    static void handleWifi();
    static void handleWifiSave();
    static void handleNotFound();
    static void handleOnOff();
    static void handleOnOffSave();
    static void handleOta();
    static void IRAM_ATTR delayMicroseconds2(uint32_t us);
protected:
    void _loadCredentials();
    void _saveCredentials();
    void _connectWifi();
    static boolean _isIp(String str);
    static String _toStringIp(IPAddress ip) ;
    const __FlashStringHelper * _start_html(int width=640);
    const __FlashStringHelper * _end_html();
    boolean _captivePortal();
    void  force_ap();
    
protected:
    DNSServer*           _dns_srv = nullptr;
    ESP8266WebServer*    _esp_srv = nullptr; //(80);
    IPAddress*           _ip_addr = nullptr;    //(10, 5, 5, 1);
    IPAddress*           _net_mask = nullptr;   //(255, 255, 0, 0);
    boolean              _b_conn2wifi = false;
    boolean              _bsta = false;
    struct {
        byte                   sig;
        char                 _cur_ssid_name[32]; // = "marius";
        char                 _cur_ssid_pswd[32]; // = "myssidpass";
        char                 _ipstatic[32];
        char                 _ntpsrv[32] = "ca.pool.ntp.org";
        int                  _ontimemin=0;
        int                  _offtimemin=0;
        int                  _timezone=-5;
    
    } _eprom;

    unsigned long       _last_conn = 0;
    unsigned int        _wlan_status = WL_IDLE_STATUS;
    bool                _otaing = false;
    unsigned int        _loop = 0;
    uint32_t            _blink_rate = OFF_BLINK;
    uint32_t            _blinktime = 0;
    byte                _toggle = 0;
    byte                _ip[4];

};

#endif
