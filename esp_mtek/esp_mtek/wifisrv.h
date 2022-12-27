#ifndef WIFISRV_H
#define WIFISRV_H

#include "_dev_config.h"

class simple_clock_t;
class wifisrv_t
{
public:
    wifisrv_t();

    void begin();
    void end();
    void loop();
    int mode()const{
        if(_cli) return 50;
        if(_ap) return 25;
        return 10;
   }
    void on_relay_event(const unidata_t* );
    static const String end_htm();
    static const __FlashStringHelper * start_htm(bool content=true);
private:
    static void handleUser();
    static void handleRoot();
    static void handleWifi();
    static void handleOta();
    static void handleTime();
    static void handleSave();

    static String _toStringIp(const IPAddress& ip);
    static bool _isIp(const String& str);
    static bool _capturePage();
    void _schedule(const char* value=nullptr);

    void    _connect_cli();
    void    _enable_ap();

private:
    ESP8266WebServer    _esp;
    bool                _ap = false;
    bool                _cli = false;


    size_t              _last_remote_get = 0;
    int                 _wstate = -1;
    size_t              _con_time = 0;
    int                 _con_retry = 0;
    bool                _cli_lost = false;
public:
    static wifisrv_t*          WifiSrv;
};

#define SSID_CONF()    (CFG(ip[0]) && CFG(ssid[0]))


#endif // WIFISRV_H
