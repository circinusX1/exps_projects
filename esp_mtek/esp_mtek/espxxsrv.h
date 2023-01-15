#ifndef WIFISRV_H
#define WIFISRV_H

#include "_my_config.h"
#include "_utils.h"

class simple_clock_t;
class sensor_th_t;
class espxxsrv_t
{
public:
    espxxsrv_t();
    ~espxxsrv_t(){};

    void begin();
    void end();
    void loop();
    int mode()const{
        if(_sta_lost) return 1;
        if(_sta) return 6;
        if(_ap) return 3;
        return 10;
    }
    bool is_otaing()const{return _otaing;}
    void on_relay_event(const sensdata_t* );
    void on_sensor_event(const sensor_th_t* );
    static void  end_htm(String& page);
    static const String& start_htm(bool content=true);
    bool  sta_connected(){return _sta && _sta_lost==0;}
    bool  ap_active(){return _ap;}
private:
    static void handleUser();
    static void handleCss();
    static void handleRoot();
    static void handleWifi();
    static void handleOta();
    static void handleTime();
    static void handleWifiSave();
    static void handleFetch();
    static void handleConfig();
    static void handleConfigSave();
    static void handleError();

    static String _toStringIp(const IPAddress& ip);
    static bool _isIp(const String& str);
    static bool _capturePage();
    void _schedule(const char* value=nullptr);
    void _handleFetch();
    void _connect_cli();
    void _enable_ap();

private:
    ESP8266WebServer    _esp;
    bool                _ap = false;
    bool                _sta = false;
    size_t              _last_remote_get = 0;
    int                 _wstate = -1;
    size_t              _con_time = 0;
    int                 _con_retry = 0;
    int                 _delay = 0;
    bool                _otaing=false;
    int                 _sta_lost = 0;
    bool                _began = false;
public:
    static espxxsrv_t*          WifiSrv;
};

#define SSID_CONF()    (CFG(ip[0]) && CFG(ssid[0]))


#endif // WIFISRV_H
