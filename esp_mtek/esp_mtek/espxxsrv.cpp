#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include "libraries/ESP8266Ping/src/ESP8266Ping.h"
#include "espxxsrv.h"
#include "eeprom.h"
#include "clock_t.h"
#include "application.h"

/////////////////////////////////////////////////////////////////////////////////////////
static const unsigned long CON_RETRY_SEC   = 30;    // wifi client losy con retry
static const unsigned long MAX_FAILS       = 10;    // max retry client then goes to AP

/////////////////////////////////////////////////////////////////////////////////////////
espxxsrv_t*  espxxsrv_t::WifiSrv;

espxxsrv_t::espxxsrv_t():_esp(80)
{
    LOG("CREATING ESP SRV_T");
    espxxsrv_t::WifiSrv = this;

    static IPAddress ip;   ip.fromString("10.5.5.1");
    static IPAddress mask; mask.fromString("255.255.0.0");
    LOG("Starting AP");
    WiFi.softAPConfig(ip, ip, mask);
    WiFi.softAP("minitk", "MINITEK0");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    _esp.onNotFound(espxxsrv_t::handleUser);
    _esp.on("/", espxxsrv_t::handleRoot);
    _esp.on("/index.html", espxxsrv_t::handleRoot);
    _esp.on("/fetch", espxxsrv_t::handleFetch);
    _esp.on("/wifi", espxxsrv_t::handleWifi);
    _esp.on("/error", espxxsrv_t::handleError);
    _esp.on("/wifisave", espxxsrv_t::handleWifiSave);
    _esp.on("/config", espxxsrv_t::handleConfig);
    _esp.on("/configsave", espxxsrv_t::handleConfigSave);
    _esp.on("/ota", espxxsrv_t::handleOta);
    _esp.on("/time", espxxsrv_t::handleTime);
    _esp.on("/fileup", HTTP_POST, []() {
        espxxsrv_t::WifiSrv->_otaing=false;
        if(Update.hasError())
            espxxsrv_t::WifiSrv->_esp.sendHeader("Location","/error");
        else
            espxxsrv_t::WifiSrv->_esp.sendHeader("Location","/");
        espxxsrv_t::WifiSrv->_esp.send(300);
        LOG("REBOOTING");
        ESP.restart();
    }, []() {
        HTTPUpload& upload = espxxsrv_t::WifiSrv->_esp.upload();
        espxxsrv_t::WifiSrv->_otaing=true;
        if (upload.status == UPLOAD_FILE_START)
        {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(MAX_PROGRAM_SZ))
            {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE)
        {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
            {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END)
        {
            if (Update.end(true))
            {
                Serial.printf("Update Success: %u\r\nRebooting...\r\n", upload.totalSize);
                // ESP.restart();
            }
            else
            {
                Update.printError(Serial);
            }
        }
    });
}

/////////////////////////////////////////////////////////////////////////////////////////
void  espxxsrv_t::end()
{
    _sta = _ap = false;
    WiFi.softAPdisconnect (true);
    WiFi.disconnect();
}

/////////////////////////////////////////////////////////////////////////////////////////
void  espxxsrv_t::begin()
{
    LOG("WIFI BEGIN.....");
    _esp.begin();
    delay(32);
    _ap  = false;
    _sta = false;
    _con_time = 0;
    _sta_lost = 0;
    _con_time = Sclk.seconds();
    LOG(__FUNCTION__);
    if(SSID_CONF())
    {
        LOG("CONNECTING CLI");
        _connect_cli();
    }
    else
    {
        LOG("STA SETTINGS MISSING");
        _sta_lost = MAX_FAILS;
    }
    _began = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::loop()
{
    if(!_began){
        LOG("LOOPING NOT BEGAN");
        return;
    }
    if(_sta_lost>0)
    {
        if(Sclk.diff_time(Sclk.seconds(),_con_time) > CON_RETRY_SEC)
        {
            --_sta_lost;
            LOG("STA LOST STA-LOST=%d",_sta_lost);
            if(_sta_lost>0)
            {
                LOG("RE-CONNECT CLI");
                _connect_cli();
                _sta = true;
            }
            else
            {
                LOG("STARTING AP");
                _enable_ap();
                _sta_lost = 0;
                _sta = false;
            }
            _con_time = Sclk.seconds();
        }
    }

    int s = WiFi.status();
    if(s != _wstate)
    {
        LOG("WIFI STATE CHANGED = %d", s);
        _wstate = s;
        if(_sta)
        {
            LOG("STA MODE %d", s);
            if (s != WL_CONNECTED)
            {
                WiFi.disconnect();
                if(_sta_lost==0)
                {
                    LOG("STA LOST, RETRYING FROM %d", MAX_FAILS);
                    _sta_lost = MAX_FAILS;
                }
            }
            else
            {
                LOG("STA CONNECTED");
                _sta_lost = 0;
                _sta = true;
            }
        }
        // no  config
        if(!SSID_CONF())
        {
            if(_ap==false)
            {
                LOG("AP AP AP");
                _enable_ap();
                _ap        = true;
                _sta       = false;
                _sta_lost  = 0;
            }
        }
    }
    _esp.handleClient();
    if(_otaing==false && sta_connected())
    {
        _schedule();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::_connect_cli()
{
    if(SSID_CONF())
    {
        LOG("Connecting to: %s with: %s", CFG(ssid), CFG(ip));
        if(WiFi.status() == WL_CONNECTED){ WiFi.disconnect();}
        TRACE();
        WiFi.begin(CFG(ssid), CFG(passwd));
        IPAddress ip; ip.fromString(CFG(ip));
        IPAddress gw; gw.fromString(CFG(gw));
        IPAddress mask;     mask.fromString(CFG(subnet));
        LOG("DNS=%s DNS=%s", CFG(dnsa), CFG(dnsb));
        IPAddress dns1;     dns1.fromString(CFG(dnsa));
        IPAddress dns2;     dns2.fromString(CFG(dnsb));
        WiFi.hostname("minitek");
        TRACE();
        _sta = true;
        if (!WiFi.config(ip, gw, mask, dns1, dns2))
        {
            TRACE();
            goto WIFI_STA_ERR;
        }
        else
        {
            TRACE();
            _wstate = WiFi.waitForConnectResult();
            LOG("Connection Result: %d", _wstate);
            Serial.printf("ip:%s gw:%s sn:%s dns:%s dns:%s \r\n", CFG(ip), CFG(gw),
                          CFG(subnet), CFG(dnsa), CFG(dnsb));
            if(_wstate == WL_CONNECTED)
            {
                LOG("CONNECTED to: %s with: %s", CFG(ssid), CFG(ip));
                _con_retry = 0;
                if(_ap)
                {
                    if(CFG(keepsta)==false)
                    {
                        LOG("Closing AP 10.5.5.1 minitk");
                        WiFi.softAPdisconnect (true);
                        _ap = false;
                    }
                    else
                    {
                        LOG("Keeping AP 10.5.5.1 minitk");
                        _ap = true;
                    }
                }
                WiFi.mode(WIFI_STA);
                Serial.print("DNSS: ");
                WiFi.dnsIP().printTo(Serial);
                Serial.print(", ");
                WiFi.dnsIP(1).printTo(Serial);
                Serial.println();
                delay(100);
                _sta=true;
                _sta_lost = 0;
            }
            else
            {
                goto WIFI_STA_ERR;
            }
        }
    }
    TRACE();
    return;
WIFI_STA_ERR:
    TRACE();
    if(_sta_lost!=0)
    {
        LOG("CANNOT CONNECT TO %s, RETRYING", CFG(ssid));
    }
    else
    {
        LOG("CANNOT CONNECT TO %s, GOING AP", CFG(ssid));
    }
    if(WiFi.status() == WL_CONNECTED){ WiFi.disconnect();}
    _wstate = -1;
    TRACE();
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::_enable_ap()
{
    LOG("ENABLING mteck AP. Connect to 10.5.5.1");
    if(WiFi.status() == WL_CONNECTED){ WiFi.disconnect();}
    delay(128);
    WiFi.enableAP(true);
    delay(128);
    WiFi.mode(WIFI_AP);
    _sta=false;
    _ap=true;
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::_schedule(const char* value)
{
    return;
    if(_wstate!=WL_CONNECTED)
    {
        return;
    }
    else{
        eeprom_t e;

        size_t diff = Sclk.diff_time(Sclk.seconds(),_last_remote_get);
        if(value || diff > (CFG(get_interval) + _delay))
        {
            LOG("CLIENT SCHEDULE FOR VALUE = %s", value ? value : "NULL");
            _last_remote_get = Sclk.seconds();

            if(e->h_get[0])                                 /// remote commands
            {
                strcpy(e->h_host,"enjoydecor.com");
                e->h_finger[0]=0;
                e->h_port=80;

                String              line,url;
                WiFiClient*         pclient = nullptr;
                const char*         host = e->h_host;
                short               nport = 80, np;
                HTTPClient          https;

                ESP.resetFreeContStack();

                if(e->h_finger[0] && host[0])
                {
                    LOG("WiFiClientSecure------------------");
                    pclient = new BearSSL::WiFiClientSecure();
                    char finger[64] = {0};
                    for(int i=0;e->h_finger[i];i++)
                    {
                        if(e->h_finger[i]==':')finger[i]=' ';
                        else finger[i]=e->h_finger[i];
                    }
                    nport = 443;
                    if(e->h_finger[0])
                    {
                        Serial.printf("FP: %s\r\n", finger);
                        if (!((WiFiClientSecure*)pclient)->setFingerprint(e->h_finger))
                        {
                            LOG("failed to finger cert %s",host);
                            goto DIONE;
                        }
                    }
                }
                else
                {
                    LOG("WiFiClient      ------------------");
                    pclient = new WiFiClient();
                    nport = 80;
                }
                np =  e->h_port ? e->h_port : nport;
                url = host;  if(!url.endsWith("/")) url+="/";
                if(value == nullptr)                           // we get if any is remote
                {
                    url += e->h_get;
                    if(url.endsWith("/"))  url+="?RELAY=";
                    else if(url.indexOf('&')!=-1) url+="&RELAY=";
                    url += String(app_t::TheApp->get_relay());
                }
                else
                {
                    url += e->h_put;
                    if(url.endsWith("/"))  url+="?";
                    else if(url.indexOf('&')!=-1) url+="&";
                    url += value;
                }
                Serial.println(url.c_str());
                LOG(" on %s port %d", e->h_host, np);
                if(https.begin(*pclient, e->h_host, np, url))
                {
                    int httpCode = https.GET();
                    if (httpCode > 0) {
                        // HTTP header has been send and Server response header has been handled
                        Serial.printf("[HTTPS] GET... code: %d\r\n", httpCode);

                        // file found at server
                        if (httpCode == HTTP_CODE_OK ||
                                httpCode == HTTP_CODE_MOVED_PERMANENTLY)
                        {
                            String payload = https.getString();
                            Serial.println(payload);
                        }
                        _delay=0;
                    } else {
                        Serial.printf("connect error: %s\r\n", https.errorToString(httpCode).c_str());
                        bool ret = Ping.ping(e->h_host);
                        LOG("PING %s = %d", e->h_host, ret);
                        _delay=30;
                    }
                    https.end();
                }
DIONE:
                pclient->stop();
                uint32_t freeStackEnd = ESP.getFreeContStack();
                delete pclient;
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::on_relay_event(const sensdata_t* value)
{
    char svalue[32];

    ::sprintf(svalue,"RELAY=%d",int(value->u.uc));
    _schedule(svalue);
}

//////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::on_sensor_event(const sensor_th_t* sensor)
{
#if I2C_SDA
    char    svalue[32];
    const   th_t& th = sensor->get_th();
    ::sprintf(svalue,"TEMP=%f4.2&HUM=%4.2f",th.t, th.h);
    _schedule(svalue);
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
// time:Relay:Temp:Hum:Temp:Hum...
void espxxsrv_t::_handleFetch()
{
#if I2C_SDA
    String page=espxxsrv_t::start_htm(false);
    page += String( Sclk.minutes())+":";
    page += String(app_t::TheApp->get_relay())+":";
    const sensors_t& ss =  app_t::TheApp->sensors();
    for(int s = 0; s< MAX_SENS; s++)
    {
        const sensor_th_t* th = ss.sensor(s);
        if(th)
        {
            const th_t& thh =  th->get_th();
            page += String(int(thh.t))+":";
            page += String(int(thh.h))+":";
        }
    }
    _esp.send(200, "text/html", page);
#endif
}
