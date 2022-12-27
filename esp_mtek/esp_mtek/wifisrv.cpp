#include "wifisrv.h"
#include "eeprom.h"
#include "clock_t.h"
#include "application.h"

static const unsigned long CON_RETRY_SEC   = 30;
static const unsigned long MAX_FAILS = 5;
/////////////////////////////////////////////////////////////////////////////////////////
wifisrv_t*  wifisrv_t::WifiSrv;

wifisrv_t::wifisrv_t():_esp(80)
{

    LOG(__FUNCTION__);
    wifisrv_t::WifiSrv = this;

    static IPAddress ip;   ip.fromString("10.5.5.1");
    static IPAddress mask; mask.fromString("255.255.0.0");
    LOG("SOFT AIP CONFIG");
    WiFi.softAPConfig(ip, ip, mask);
    WiFi.softAP("minitk", "MINITEK0");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    _esp.onNotFound(wifisrv_t::handleUser);
    _esp.on("/", wifisrv_t::handleRoot);
    _esp.on("/index.html", wifisrv_t::handleRoot);
    _esp.on("/wifi", wifisrv_t::handleWifi);
    _esp.on("/ota", wifisrv_t::handleOta);
    _esp.on("/time", wifisrv_t::handleTime);
    _esp.on("/wifisave", wifisrv_t::handleSave);
    _esp.on("/fileup", HTTP_POST, []() {
        LOG("FILEUP1");
        wifisrv_t::WifiSrv->_esp.sendHeader("Connection", "close");
        wifisrv_t::WifiSrv->_esp.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();

    }, []() {
        HTTPUpload& upload = wifisrv_t::WifiSrv->_esp.upload();
        if (upload.status == UPLOAD_FILE_START) {
            TRACE();
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(MAX_PROGRAM_SZ))
            {
                Update.printError(Serial);
            }
            TRACE();
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            TRACE();
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
            TRACE();
        } else if (upload.status == UPLOAD_FILE_END) {
            TRACE();
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\r\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
            TRACE();
        }
    });

}

/////////////////////////////////////////////////////////////////////////////////////////
void  wifisrv_t::end()
{
    _cli = _ap = false;
    WiFi.softAPdisconnect (true);
    WiFi.disconnect();
}

/////////////////////////////////////////////////////////////////////////////////////////
void  wifisrv_t::begin()
{
    _esp.begin();
    _ap  = false;
    _cli = false;
    _cli_lost = false;
    _con_time = 0;
    _connect_cli();
}

/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::loop()
{
    if((_cli_lost && Sclk.diff_time(Sclk.seconds(),_con_time) > CON_RETRY_SEC))
    {
        _con_time = Sclk.seconds();
        _connect_cli();
    }

    int s = WiFi.status();
    if(s != _wstate)
    {
        LOG("WIFI STATE = %d", s);
        _wstate = s;
        if(_cli)
        {
            if (s != WL_CONNECTED)
            {
                _cli      = false;
                _cli_lost = true;
                _con_time = Sclk.seconds();
            }
        }
        // no  config
        if(!SSID_CONF() && _ap==false)
        {
            _enable_ap();
            _ap       = true;
            _cli      = false;
            _cli_lost = false;
        }
    }
    _esp.handleClient();
    _schedule();
}

/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::_connect_cli()
{
    if(SSID_CONF())
    {
        LOG("Connecting to: %s with: %s", CFG(ssid), CFG(ip));

        _cli_lost = false;

        WiFi.disconnect();
        WiFi.begin(CFG(ssid), CFG(passwd));
        IPAddress ip; ip.fromString(CFG(ip));
        IPAddress gw; gw.fromString(CFG(gw));
        IPAddress mask; mask.fromString(CFG(subnet));
        IPAddress dns1; dns1.fromString(CFG(dns[0]));
        IPAddress dns2; dns2.fromString(CFG(dns[1]));
        if (!WiFi.config(ip, gw, mask, dns1, dns2))
        {
            LOG("CLIENT Failed to configure. Going on AP");
            _cli  = false;
            WiFi.disconnect();
            _wstate = -1;
        }
        else
        {
            _wstate = WiFi.waitForConnectResult();
            LOG("Connection Result: %d", _wstate);
            if(_wstate == WL_CONNECTED)
            {
                LOG("CONNECTED to: %s with: %s", CFG(ssid), CFG(ip));
                _con_retry = 0;
                _cli_lost = false;
                if(CFG(keepsta)==false && _ap)
                {
                    LOG("Closing AP");
                    WiFi.softAPdisconnect (true);
                    _ap = false;
                }
            }
            else
            {
                LOG("CANNOT CONNECT TO %s", CFG(ssid));
                _cli  = false;
                WiFi.disconnect();
                _wstate = -1;
            }
        }
        CFG(ssid[0]=0);
        CFG(ip[0]=0);
    }

}

/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::_enable_ap()
{
    LOG("ENABLING AP");
    WiFi.disconnect();
    delay(128);
    WiFi.enableAP(true);
    delay(128);
}


/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::_schedule(const char* value)
{
    if(_wstate!=WL_CONNECTED)
    {
        return;
    }
    else{
        eeprom_t e;

        size_t diff = Sclk.diff_time(Sclk.seconds(),_last_remote_get);
        if(value || diff > 10)
        {
            _last_remote_get = Sclk.seconds();

            LOG("WGET at %d %s", Sclk.seconds(), value ? value : "empty");
            if(e->h_get[0])                                 /// remote commands
            {
                String line,url;
                WiFiClient* pclient = nullptr;
                const char* host = e->h_host;
                short nport = 80;
                if(e->h_finger[0] && host[0])
                {
                    LOG("SECURE");
                    pclient = new WiFiClientSecure();
                    char finger[64] = {0};
                    for(int i=0;e->h_finger[i];i++)
                    {
                        if(e->h_finger[i]==':')finger[i]=' ';
                        else finger[i]=e->h_finger[i];
                    }
                    nport = 443;
                }
                else
                {
                    LOG("HTTP connect");
                    pclient = new WiFiClient();
                }

                if(e->h_finger[0])
                {
                    if (!((WiFiClientSecure*)pclient)->setFingerprint(e->h_finger))
                    {
                        LOG("failed to finger cert %s",host);
                        goto DIONE;
                    }
                }
                if(!pclient->connect(host, e->h_port ? e->h_port : nport))
                {
                    LOG("failed to connect to %s:%d",host,e->h_port?e->h_port:nport);
                    goto DIONE;
                }


                url = host;
                if(value == nullptr)                                // we get if any is remote
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

                pclient->print(String("GET ") + url + " HTTP/1.1\r\n" +
                               "Host: " + host + "\r\n" +
                               "User-Agent: esp_mtek\r\n" +
                               "Connection: close\r\n\r\n");

                while (pclient->connected())
                {
                    String line = pclient->readStringUntil('\n');
                    if (line == "\r") {
                        break;
                    }
                }
                line = pclient->readStringUntil('\n');
                if(line.length())
                {
                    LOG("RECEIVED %d", line);
                    if(value == nullptr)
                    {
                        unidata_t* pd = unidata_t::get(eRELAY, line.toInt());
                        app_t::TheApp->on_web_event(pd);
                    }
                }
DIONE:
                pclient->stop();
                delete pclient;
            }
        }
    }
}

void wifisrv_t::on_relay_event(const unidata_t* value)
{
}


