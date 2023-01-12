#include "_my_config.h"
#include "eeprom.h"
#include "application.h"
#include "espxxsrv.h"




bool espxxsrv_t::_capturePage()
{
    eeprom_t e;

    if (!_isIp(espxxsrv_t::WifiSrv->_esp.hostHeader()) &&
            espxxsrv_t::WifiSrv->_esp.hostHeader() != "minitk.local")
    {
        Serial.println("Request redirected to captive portal");
        espxxsrv_t::WifiSrv->_esp.sendHeader("Location", String("http://") +
                                             _toStringIp(espxxsrv_t::WifiSrv->_esp.client().localIP()), true);
        espxxsrv_t::WifiSrv->_esp.send(302, "text/plain", "");
        delay(100);
        espxxsrv_t::WifiSrv->_esp.client().stop();
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleConfig()
{
    String page = espxxsrv_t::start_htm();

    page += F("<form method='POST' action='configsave'><h4>DEVICE CONFIG</h4>");

    page += F("<li>schedule     <input type='text' name='schedule' value='")+ _S(CFG(schedule)) + F("' >");
    page += F("<li>autoenabled  <input type='text' name='autoenabled' value='")+ _S(CFG(autoenabled)) + F("' >");
    page += F("<li>keepsta      <input type='text' name='keepsta' value='")+ _S(CFG(keepsta)) + F("' >");
    page += F("<li>temptrig     <input type='text' name='temptrig' value='")+ _S(CFG(temptrig)) + F("' >");
    page += F("<li>humtrig      <input type='text' name='humtrig' value='")+ _S(CFG(humtrig)) + F("' >");
    page += F("<li>trigger rule <select name='triggerrule'>");
    page += F("<option value=1>Temp above and Humidity above</option>");
    page += F("<option value=2>Temp above and Humidity below</option>");
    page += F("<option value=3>Temp below and Humidity above</option>");
    page += F("<option value=4>Temp below and Humidity below</option>");
    page += F("<option value=5>Temp above OR Humidity above</option>");
    page += F("<option value=6>Temp above OR Humidity below</option>");
    page += F("<option value=7>Temp below OR Humidity above</option>");
    page += F("<option value=8>Temp below OR Humidity below</option>");
    page += F("</select>");
    page += F("<li>h_host       <input type='text' name='h_host ' value='")+ _S(CFG(h_host)) + F("' >");
    page += F("<li>h_put        <input type='text' name='h_put ' value='")+ _S(CFG(h_put)) + F("' >");
    page += F("<li>h_get        <input type='text' name='h_get ' value='")+ _S(CFG(h_get)) + F("' >");
    page += F("<li>h_finger     <input type='text' name='h_finger ' value='")+ _S(CFG(h_finger)) + F("' >");
    page += F("<li>h_port       <input type='text' name='h_port ' value='")+ _S(CFG(h_port)) + F("' >");
    page += F("<li>mq_ttbroker  <input type='text' name='mq_ttbroker ' value='")+ _S(CFG(mq_ttbroker)) + F("' >");
    page += F("<li>mq_topic     <input type='text' name='mq_topic ' value='")+ _S(CFG(mq_topic)) + F("' >");
    page += F("<li>mq_user      <input type='text' name='mq_user ' value='")+ _S(CFG(mq_user)) + F("' >");
    page += F("<li>mq_pass      <input type='text' name='mq_pass ' value='")+ _S(CFG(mq_pass)) + F("' >");
    page += F("<li>mq_port      <input type='text' name='mq_port ' value='")+ _S(CFG(mq_port)) + F("' >");
    page += F("<li>get_interval <input type='text' name='get_interval' value='")+ _S(CFG(get_interval)) + F("' >");
    page += F("<li>ntp_offset   <input type='text' name='ntp_offset ' value='")+ _S(CFG(ntp_offset)) + F("' >");
    page += F("<li>ntp_srv      <input type='text' name='ntp_srv ' value='")+ _S(CFG(ntp_srv)) + F("' >");
    page += F("<li>ip           <input type='text' name='ip ' value='")+ _S(CFG(ip)) + F("' >");
    page += F("<li>gw           <input type='text' name='gw ' value='")+ _S(CFG(gw)) + F("' >");
    page += F("<li>subnet       <input type='text' name='subnet ' value='")+ _S(CFG(subnet)) + F("' >");
    page += F("<li>dnsa         <input type='text' name='dnsa ' value='")+ _S(CFG(dnsa)) + F("' >");
    page += F("<li>dnsb         <input type='text' name='dnsb ' value='")+ _S(CFG(dnsb)) + F("' >");
    page += F("<li>ssid         <input type='text' name='ssid ' value='")+ _S(CFG(ssid)) + F("' >");
    page += F("<li>passwd       <input type='passw name='passwdord' value='")+ _S(CFG(passwd)) + F("' >");
    page += F("<li>minitek      <input type='text' name='minitek ' value='")+_S(CFG(minitek)) + F("' >");
    page += F("<li>             <input type='submit name='apply' value='Apply'/></form>");
    page += espxxsrv_t::end_htm();
    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

//////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleConfigSave()
{
    String page;



    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

//////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleUser()
{
    String page;
    app_t::TheApp->http_get(&WifiSrv->_esp, page);
    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

//////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleTime()
{
    char timp[16] = {0};
    espxxsrv_t::WifiSrv->_esp.arg("time").toCharArray(timp,14);
    LOG("Setting time as web to %d", timp);
    sensdata_t* at = sensdata_t::get(eTIME, ::atoi(timp));
    app_t::TheApp->on_web_event(at);
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleOta()
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
            "   window.location.href ='/';\n"
            "}\n"
            "function ls(event) {\n"
            "    setTimeout(redir, 32000) ;\n"
            "}\n"
            "const form = document.getElementById('form');\n"
            "form.addEventListener('submit', ls);\n"
            "</script>\n";
    page+=espxxsrv_t::WifiSrv->end_htm();
    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

/////////////////////////////////////////////////////////////////////////////////////////
void espxxsrv_t::handleWifi()
{
    eeprom_t    e;
    String      page = espxxsrv_t::WifiSrv->start_htm();
    IPAddress   sta; sta.fromString("10.5.5.1");

    if (espxxsrv_t::WifiSrv->_esp.client().localIP() == sta) {
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
            String(e->ntp_offset)+
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
                "<br /><input type='text' placeholder='IP: 192.168.1.178' name='i'/>"
                "<br /><input type='text' placeholder='GATEWAY: 192.168.1.1' name='i'/>"
                "<br /><input type='text' placeholder='Time Zone in hours' name='z'/>"
                "<br /><input type='submit' value='Apply'/></form>");
    page += espxxsrv_t::WifiSrv->end_htm();

    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
    espxxsrv_t::WifiSrv->_esp.client().stop(); // Stop is needed because we sent no content length
}

void espxxsrv_t::handleWifiSave()
{
    eeprom_t e(1);

    char ssid[32]={0};
    char passwd[32]={0};
    char ip[24]={0};
    char gw[24]={0};
    char time[24]={0};

    espxxsrv_t::WifiSrv->_esp.arg("n").toCharArray(ssid, sizeof(ssid) - 1);
    espxxsrv_t::WifiSrv->_esp.arg("p").toCharArray(passwd, sizeof(passwd) - 1);
    espxxsrv_t::WifiSrv->_esp.arg("i").toCharArray(ip, sizeof(ip) - 1);
    espxxsrv_t::WifiSrv->_esp.arg("g").toCharArray(gw, sizeof(gw) - 1);
    espxxsrv_t::WifiSrv->_esp.arg("z").toCharArray(time, sizeof(time) - 1);

    if(ssid[0])     ::strcpy(e->ssid, ssid);
    if(passwd[0])   ::strcpy(e->passwd, passwd);
    if(ip[0])       ::strcpy(e->ip, ip);
    if(gw[0])       ::strcpy(e->gw, gw);
    if(time[0])     e->ntp_offset = ::atoi(time);

    LOG("FORM VALUES %s %s %s %s %d", e->ssid, e->passwd, e->ip, e->gw, e->ntp_offset);
    e.save();
    delay(200);
    e.load();
    String page = espxxsrv_t::start_htm();
    page += F("<li>SAVING:");
    page += F("<li> SSID:")   + _S(CFG(ssid));
    page += F("<li> IP:")     + _S(CFG(ip));
    page += F("<li> GW:")     + _S(CFG(gw));
    page += F("<li> OFFSET:") + _S(CFG(ntp_offset));

    page += F("<li>Wait 30 seconds, join your router and click ");
    page += F("<a href='http://") + String(e->ip) + F("'>MINITK</a>");
    page += espxxsrv_t::end_htm();
    espxxsrv_t::WifiSrv->_esp.send(200, F("text/html"), page);
    espxxsrv_t::WifiSrv->_esp.client().stop(); // Stop is needed because we sent no content length
    ESP.restart();
}

void espxxsrv_t::handleRoot()
{
    LOG("HANDLE ROOT");
    if (espxxsrv_t::WifiSrv->_capturePage())
    {
        return;
    }
    String page;
    app_t::TheApp->http_get(&espxxsrv_t::WifiSrv->_esp,page);
    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", page);
}

const String espxxsrv_t::end_htm()
{
    return F("</div></body></html>");
}

const String& espxxsrv_t::start_htm(bool content)
{
    static String ret;

    ret = "";
    espxxsrv_t::WifiSrv->_esp.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    espxxsrv_t::WifiSrv->_esp.sendHeader("Pragma", "no-cache");
    espxxsrv_t::WifiSrv->_esp.sendHeader("Expires", "-1");
    if(content)
    {
        ret = F("<!DOCTYPE html><html lang='en'>\n<head>\n"
                "<meta name='viewport' content='width=device-width'>\n"
                "<style>.cent{ margin: auto;"
                "width: 96%; text-align:center;font-size:1em;"
                "border: 3px solid red; background:#DDF;"
                "padding: 6px;margin-top:80px;}"
                "table{ border: 1px solid; width:100%;}"
                ".menu{text-align:center; fint-size:1.5em; color:#522; "
                "background-color: #c0c0e0; position:fixed; top:0; width:100%; z-index:100;}"
                "button,input[type='submit']{background-color:#8EF;color:black;width:160px;height:40px;}"
                ".but{display:inline-flex;padding-top:9px;background-color:#8EF;color:black;width:160px;height:28px;}"
                "input[type='file']{background-color:#8EF;color:black;width:160px;height:60px;display:none}"
                "</style>\n"
                "<title>MARIUTEK</title></head>\n<body>\n"
                "<div class='menu'><div align='left'>")+
                String(BTIME)+
                F("</div><a href='/'><button>MAIN PAGE</button></a>"
                  "<a href='/ota'><button>UPDATE</button></a> "
                  "<a href='/wifi'><button>WIFI CONFIG</button></a>"
                  "<a href='/config'><button>CONFIG</button></a> "
                  "</div>"
                  "<div class='cent'>\n");
    }
    return ret;
}


bool espxxsrv_t::_isIp(const String& str)
{
    for (size_t i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c !='.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String espxxsrv_t::_toStringIp(const IPAddress& ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

void espxxsrv_t::handleFetch()
{
    espxxsrv_t::WifiSrv->_handleFetch();
}

void espxxsrv_t::handleError()
{
    espxxsrv_t::WifiSrv->_esp.send(200, "text/html", "Error OTA");
}
