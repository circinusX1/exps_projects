#ifndef AP_WIFI_OTA
#define AP_WIFI_OTA


#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>


#define SAMPLES_MAX  2880

struct Temps
{
    int   _count;
    float _data[SAMPLES_MAX];
};


class esp32_full
{
public:  
    esp32_full(byte, byte, byte, byte, int);
    virtual ~esp32_full();

    bool    setup();
    bool    loop();
    virtual void  user_loop(unsigned int) = 0;
    virtual void  page_request(const String getput,
                               const String& path,
                               String& page)=0;

public:
    static void handleRoot();
    static void handleWifi();
    static void handleWifiSave();
    static void handleNotFound();
    static void handleOta();
    static void IRAM_ATTR delayMicroseconds2(uint32_t us);
protected:
    void _loadCredentials();
    void _saveCredentials();
    void _connectWifi();
    static boolean _isIp(String str);
    static String _toStringIp(IPAddress ip) ;
    
    boolean _captivePortal();

protected:
    DNSServer*           _dns_srv = nullptr;
    ESP8266WebServer*    _esp_srv = nullptr; //(80);
    IPAddress*           _ip_addr = nullptr;    //(10, 5, 5, 1);
    IPAddress*           _net_mask = nullptr;   //(255, 255, 0, 0);
    boolean              _b_connect = false;
    char                 _cur_ssid_name[33]; // = "marius";
    char                 _cur_ssid_pswd[65]; // = "myssidpass";

    unsigned long       _last_conn = 0;
    unsigned int        _wlan_status = WL_IDLE_STATUS;
    bool                _otaing = false;
    unsigned int        _loop = 0;
    unsigned int        _blink = 10;
    byte                _toggle = 0;
    byte                _ip[4];

};

#endif
