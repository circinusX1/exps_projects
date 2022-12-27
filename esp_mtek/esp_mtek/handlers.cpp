#include "_dev_config.h"
#include "eeprom.h"
#include "application.h"
#include "wifisrv.h"




bool wifisrv_t::_capturePage()
{
    eeprom_t e;

    if (!_isIp(wifisrv_t::WifiSrv->_esp.hostHeader()) &&
            wifisrv_t::WifiSrv->_esp.hostHeader() != "minitk.local")
    {
        Serial.println("Request redirected to captive portal");
        wifisrv_t::WifiSrv->_esp.sendHeader("Location", String("http://") +
                            _toStringIp(wifisrv_t::WifiSrv->_esp.client().localIP()), true);
        wifisrv_t::WifiSrv->_esp.send(302, "text/plain", "");
        delay(100);
        wifisrv_t::WifiSrv->_esp.client().stop();
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::handleUser()
{
    String page;
    app_t::TheApp->http_get(&WifiSrv->_esp, page);
    wifisrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

//////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::handleTime()
{
    char timp[16] = {0};
    wifisrv_t::WifiSrv->_esp.arg("time").toCharArray(timp,14);
    unidata_t* at = unidata_t::get(eTIME, ::atoi(timp));
    app_t::TheApp->on_web_event(at);
}

/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::handleOta()
{
    TRACE();
    String page = start_htm();
    page += "<div id='msg'></div><form method='POST' id='form' name='form' "
            "action='/fileup' enctype='multipart/form-data' id='upload_form'>"
            "<label class='but'><input type='file' name='update'>Select bin</label>"
            "<input type='submit'  value='Update'>"
            "</form>"
            "<script>\n"
            "function redir() {\n"
            "   window.location.href = '/';\n"
            "}\n"
            "function ls(event) {\n"
            "    setTimeout(redir, 40000) ;\n"
            "}\n"
            "const form = document.getElementById('form');\n"
            "form.addEventListener('submit', ls);\n"
            "</script>\n";
    page+=wifisrv_t::WifiSrv->end_htm();
    wifisrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

/////////////////////////////////////////////////////////////////////////////////////////
void wifisrv_t::handleWifi()
{
    eeprom_t    e;
    String      page = wifisrv_t::WifiSrv->start_htm();
    IPAddress   sta; sta.fromString("10.5.5.1");

    if (wifisrv_t::WifiSrv->_esp.client().localIP() == sta) {
        page += String(F("<p>Connected toSoft AP: minitk</p>"));
    } else {
        page += String(F("<p>Connected to Wifi network: ")) + e->ip + F("</p>");
    }
    page +=
            String(F(
                       "\r\n<br />"
                       "<table><tr><th align='left'>SoftAP config</th></tr>"
                       "<tr><td>AP minitk")) +
            F("</td></tr>"
              "<tr><td>IP ") +
            _toStringIp(WiFi.softAPIP()) +
            F("</td></tr>"
              "</table>"
              "\r\n<br />"
              "<table><tr><th align='left'>WLAN config</th></tr>"
              "<tr><td>SSID ") +
            String(e->ssid) +
            F("</td></tr>"
              "<tr><td>IP ") +
            _toStringIp(WiFi.localIP()) +
            F("</td></tr><tr><td>Time Zone:")+
            String(e->timeoff)+
            F("</td></tr></table>"
              "\r\n<br />"
              "<table><tr><th align='left'>WLAN list (if any missing predd F5)</th></tr>");
    Serial.println("scan start");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) +
                    ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") +
                    WiFi.RSSI(i) + F(")</td></tr>");
        }
    } else {
        page += F("<tr><td>No WLAN found</td></tr>");
    }
    page += F(
                "</table>"
                "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
                "<input type='text' placeholder='SSID' name='n'/>"
                "<br /><input type='text' placeholder='PASSWORD' name='p'/>"
                "<br /><input type='text' placeholder='192.168.1.178' name='i'/>"
                "<br /><input type='text' placeholder='Time Zone in hours' name='z'/>"
                "<br /><input type='submit' value='Apply'/></form>");
    page += wifisrv_t::WifiSrv->end_htm();

    wifisrv_t::WifiSrv->_esp.send(200, "text/html", page);
    wifisrv_t::WifiSrv->_esp.client().stop(); // Stop is needed because we sent no content length
}

void wifisrv_t::handleSave()
{
    eeprom_t e(1);

    Serial.println("wifi save");
    wifisrv_t::WifiSrv->_esp.arg("n").toCharArray(e->ssid, sizeof(e->ssid) - 1);
    wifisrv_t::WifiSrv->_esp.arg("p").toCharArray(e->passwd, sizeof(e->passwd) - 1);
    wifisrv_t::WifiSrv->_esp.arg("i").toCharArray(e->ip, sizeof(e->ip) - 1);
    wifisrv_t::WifiSrv->_esp.arg("z").toCharArray(e->timeoff, sizeof(e->timeoff) - 1);

    LOG("FORM VALUES %s %s %s", e->ssid, e->passwd, e->ip);
    String page = wifisrv_t::start_htm();
    page += "SAVING & REBOOTING";
    page += wifisrv_t::end_htm();
    wifisrv_t::WifiSrv->_esp.send(200, "text/html", page);
    wifisrv_t::WifiSrv->_esp.client().stop(); // Stop is needed because we sent no content length
    e.save();
    delay(200);
    REBOOT();
}

void wifisrv_t::handleRoot()
{
    LOG("HANDLE ROOT");
    if (wifisrv_t::WifiSrv->_capturePage())
    {
        return;
    }
    if(wifisrv_t::WifiSrv->_ap)
    {
        wifisrv_t::handleWifi();
    }
    else
    {
        String page;
        app_t::TheApp->http_get(&wifisrv_t::WifiSrv->_esp,page);
        wifisrv_t::WifiSrv->_esp.send(200, "text/html", page);
    }
}

const String wifisrv_t::end_htm()
{
    return F("</div></body></html>");
}

const __FlashStringHelper * wifisrv_t::start_htm(bool content)
{
    wifisrv_t::WifiSrv->_esp.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    wifisrv_t::WifiSrv->_esp.sendHeader("Pragma", "no-cache");
    wifisrv_t::WifiSrv->_esp.sendHeader("Expires", "-1");
    if(content)
    {
        return F("<!DOCTYPE html><html lang='en'>\n<head>\n"
                 "<meta name='viewport' content='width=device-width'>\n"
                 "<style>.cent{ margin: auto;"
                 "width: 800px; text-align:center;font-size:1em;"
                 "border: 3px solid red; background:#DDF;"
                 "padding: 6px;margin-top:80px;}"
                 "table, th, td { border: 1px solid; width:100%;}"
                 ".menu{text-align:center; fint-size:1.5em; color:#522; "
                 "background-color: #c0c0e0; position:fixed; top:0; width:100%; z-index:100;}"
                 "button,input[type='submit']{background-color:#8EF;color:black;width:160px;height:40px;}"
                 ".but{dislay:inline-flex;padding-top:9px;background-color:#8EF;color:black;width:160px;height:28px;}"
                 "input[type='file']{background-color:#8EF;color:black;width:160px;height:60px;display:none}"
                 "</style>\n"
                 "<title>MARIUTEK</title></head>\n<body>\n"
                 "<div class='menu'>"

                 "<button><a href='/'>MAIN PAGE</a></button>"
                 "<button><a href='/ota'>UPDATE</a> </button>"
                 "<button><a href='/wifi'>WIFI CONFIG</a> </button>"
                 "</div>"
                 "<div class='cent'>\n");
    }
    return F("");
}


bool wifisrv_t::_isIp(const String& str)
{
    for (size_t i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String wifisrv_t::_toStringIp(const IPAddress& ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}
