
#include "esp_32_base.h"

#define         FEW_HOURS       7200000
const char*     AP_SSID_NAME = "door";
const char*     AP_SSID_PASS = "doordoor";
const char*     HOST_NAME = "HVAC-1.0";
const byte      DNS_PORT = 53;
static esp_32_base_c*  This;
#define ESP_S()     This->_esp_srv

RAMM __Ramm __attribute__ ((section (".noinit")));


esp_32_base_c::esp_32_base_c()
{
    //_dns_srv = new DNSServer;
    _esp_srv = new ESP8266WebServer(80);
    _ip_addr = new IPAddress(10, 5, 5, 1);
    _net_mask = new IPAddress(255, 255, 0, 0);
    _last_conn = millis();
    This = this;
}

esp_32_base_c::~esp_32_base_c()
{
    //delete _dns_srv ;
    delete ESP_S() ;
    delete _ip_addr ;
    delete _net_mask;
}

bool    esp_32_base_c::setup()
{
    delay(100);
    Serial.begin(115200);
    Serial.println("");
    Serial.print("Starting AP: ");
    Serial.print(AP_SSID_NAME);
    Serial.print("/");
    Serial.print(AP_SSID_PASS);
    Serial.println(" @ http://10.5.5.1");
    Serial.flush();

    if(RELAY)pinMode(RELAY, OUTPUT);
    if(LED)pinMode(LED, OUTPUT);
    if (RELAY)digitalWrite(RELAY, __Ramm.relay_state);


    WiFi.softAPConfig(*_ip_addr, *_ip_addr, *_net_mask);
    WiFi.softAP(AP_SSID_NAME, AP_SSID_PASS);
    delay(400);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.flush();
    _bsta = true;
    ESP_S()->on("/", esp_32_base_c::handleRoot);
    ESP_S()->on("/index.html", esp_32_base_c::handleRoot);
    ESP_S()->on("/wifi", esp_32_base_c::handleWifi);
    ESP_S()->on("/ota", esp_32_base_c::handleOta);
    ESP_S()->on("/wifisave", esp_32_base_c::handleWifiSave);
    ESP_S()->on("/generate_204", esp_32_base_c::handleRoot);   //  Android captive portal. Maybe not needed. Might be handled by notFound handler.
    ESP_S()->on("/fwlink", esp_32_base_c::handleRoot);
    //    ESP_S()->on("/fileup", HTTP_POST, esp_32_base_c::otaUpdate);

    ESP_S()->onNotFound(esp_32_base_c::handleNotFound);

    ESP_S()->on("/fileup", HTTP_POST, []() {
        TRACE();
        ESP_S()->sendHeader("Connection", "close");
        ESP_S()->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        TRACE();
        ESP.restart();
        TRACE();
    }, []() {
        HTTPUpload& upload = ESP_S()->upload();
        if (upload.status == UPLOAD_FILE_START) {
            TRACE();
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(900000)) { //start with max available size
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
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
            TRACE();
        }
    });
    ESP_S()->begin();
    Serial.println("HTTP ESP_S() started");
    _loadCredentials();
    _b_conn2wifi = (_eprom.sig==SIG && _eprom._ipstatic[0]);
    Serial.print("bconnect=");
    Serial.println(_b_conn2wifi);

#if WITH_NTP
    if(_eprom._offset[0])
    {
        _timeClient =  new NTPClient(_ntpUDP,::atoi(_eprom._offset)*3600);
        _timeClient->begin();
    }
#endif

    _tictacker.attach(1, esp_32_base_c::tick_tack);

    this->user_begin();

    return true;
}

void esp_32_base_c::tick_tack()
{

    This->_seconds++;
    if(This->_seconds>1440){
        This->_seconds=0;
    }
}

bool    esp_32_base_c::loop()
{
    ++_loop;
    if(_otaing)
    {
        ESP_S()->handleClient();
        return true;
    }
    if (_b_conn2wifi)
    {
        Serial.println("Connecting to wifi");
        _connectWifi();
        _last_conn = millis();
        _b_conn2wifi = false;
        _bsta = false;
    }
    else if(_eprom._ipstatic[0]==0)
    {
        if(_bsta==false)
        {
            Serial.println("Disconnecting wifi client, No Static IP");
            WiFi.disconnect();
            _bsta = true;
        }
    }
    if(_bsta==false)
    {
        unsigned int s = WiFi.status();
        if (s == 0 && (millis() > (_last_conn + 60000) || millis() < _last_conn))
        {
            Serial.println("retrying to connect");
            _b_conn2wifi = true;
        }
        if (_wlan_status != s)
        {
            Serial.println("Wlan status changed: ");
            Serial.println(s);
            _wlan_status = s;
            if (s == WL_CONNECTED)
            {
                Serial.println("WIFI");
                Serial.print("Connected to ");
                Serial.println(_eprom._cur_ssid_name);
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());
                Serial.print("Shutting down AP ");
                WiFi.softAPdisconnect (true);
                _bsta = false;
#if WITH_NTP
                _timeClient->update();
                _ntptime = millis();
                _seconds = _timeClient->getHours()*60*60;
                _seconds += _timeClient->getMinutes()*60;
                _seconds += _timeClient->getSeconds();
#endif
            }
            else if (s == WL_NO_SSID_AVAIL)
            {
                Serial.println("No SSID available. Opening AP");
                WiFi.disconnect();
                WiFi.enableAP(true);
                _bsta=true;
            }
        }
        if (s == WL_CONNECTED)
        {
            _bsta = false;
#if WITH_NTP
            if(millis()  - _ntptime  > FEW_HOURS || millis()<_ntptime)
            {
                _timeClient->update();
                _ntptime = millis();
            }
            _seconds = _timeClient->getHours()*60*60;
            _seconds += _timeClient->getMinutes()*60;
            _seconds += _timeClient->getSeconds();
#endif
        }
    }while(0);

    //_dns_srv->processNextRequest();
    ESP_S()->handleClient();
    this->user_loop(_loop);


    if(millis() - _blinktime > _blink_rate && _blink_rate)
    {
        digitalWrite(LED, _toggle);
        if(WL_CONNECTED==_wlan_status)
        {
            _blink_rate = __Ramm.relay_state ? ON_BLINK : OFF_BLINK;
        }
        else
        {
            _blink_rate = AP_BLINK;
        }
        _toggle=!_toggle;
        _blinktime = millis();
    }
    if(_otaenabled)
    {
        digitalWrite(LED, HIGH);
        _blink_rate = 0;

    }
    return true;
}

void esp_32_base_c::_saveCredentials()
{
    EEPROM.begin(512);
    _eprom.sig=SIG;
    EEPROM.put(0, _eprom);
    EEPROM.commit();
    EEPROM.end();
    delay(1000);
}


void esp_32_base_c::_loadCredentials()
{
    EEPROM.begin(512);
    delay(512);
    EEPROM.get(0, _eprom);
    EEPROM.end();

    if (_eprom.sig!=SIG) {
        _eprom._ipstatic[20] = 0;
        Serial.println(_eprom.sig);
        Serial.println(_eprom._cur_ssid_name);
        Serial.println(_eprom._cur_ssid_pswd);
        Serial.println(_eprom._ipstatic);
        memset(&_eprom,0,sizeof(_eprom));
        Serial.println("eprom is empty. Join the AP and browse http://10.5.5.1");
    }else{
        Serial.println("Recovered credentials:");
        Serial.println(_eprom._cur_ssid_name);
        Serial.println(_eprom._cur_ssid_pswd);
        Serial.println(_eprom._ipstatic);
    }

}

void esp_32_base_c::_connectWifi()
{
    Serial.println("Connecting as wifi client...");
    WiFi.disconnect();
    WiFi.begin(_eprom._cur_ssid_name, _eprom._cur_ssid_pswd);


    IPAddress _ip;// = IPAddress(_ip[0],_ip[1],_ip[2],_ip[3]);
    _ip.fromString(_eprom._ipstatic);

    IPAddress _gw = IPAddress(192,168,1, 1);
    IPAddress _sn = IPAddress(255, 255, 255, 0);
    IPAddress primaryDNS(8, 8, 8, 8);           // optional
    IPAddress secondaryDNS(8, 8, 4, 4);         // optional
    if (!WiFi.config(_ip, _gw, _sn, primaryDNS, secondaryDNS)) {
        Serial.println("STA Failed to configure");
    }
    int connRes = WiFi.waitForConnectResult();
    Serial.print("connRes: ");
    Serial.println(connRes);
}


boolean esp_32_base_c::_isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String esp_32_base_c::_toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

#define NOP() asm volatile ("nop")
void IRAM_ATTR esp_32_base_c::delayMicroseconds2(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}

boolean esp_32_base_c::_captivePortal()
{
    if (!_isIp(ESP_S()->hostHeader()) &&
            ESP_S()->hostHeader() != (String(HOST_NAME) + ".local"))
    {
        Serial.println("Request redirected to captive portal");
        ESP_S()->sendHeader("Location", String("http://") +
                            _toStringIp(ESP_S()->client().localIP()), true);
        ESP_S()->send(302, "text/plain", "");
        ESP_S()->client().stop();
        return true;
    }
    return false;
}

const __FlashStringHelper * esp_32_base_c::_end_html()
{
    return F("</div></body></html>");
}

const __FlashStringHelper * esp_32_base_c::_start_html(bool content)
{
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");
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
                 "button,input[type='submit']{background-color:#AAF;color:black;width:160px;height:40px;}"
                 "input[type='file']{background-color:#AAF;color:black;width:160px;height:40px;display=none;}"
                 "</style>\n"
                 "<title>HUMIDIFIER</title></head>\n<body>\n"
                 "<div class='menu'>"

                 "<button><a href='/'>MAIN PAGE</a></button>"
                 "<button><a href='/ota'>UPDATE</a> </button>"
                 "<button><a href='/wifi'>WIFI CONFIG</a> </button>"
                 "</div>"
                 "<div class='cent'>\n");
    }
    return F("");
}
void esp_32_base_c::handleOta()
{
    TRACE();
    This->_otaenabled = true;
    String page = This->_start_html();
    page += "<form method='POST' "
            "action='/fileup' enctype='multipart/form-data' id='upload_form'>"
            "<input type='file' name='update'>"
            "<input type='submit' value='Update'>"
            "</form>";

    page+=This->_end_html();
    ESP_S()->send(200, "text/html", page);
}

void esp_32_base_c::handleWifi()
{

    This->_otaing=false;


    String Page = This->_start_html();
    if (ESP_S()->client().localIP() == *This->_ip_addr) {
        Page += String(F("<p>Connected toSoft AP: ")) + AP_SSID_NAME + F("</p>");
    } else {
        Page += String(F("<p>Connected to Wifi network: ")) + This->_eprom._cur_ssid_name + F("</p>");
    }
    Page +=
            String(F(
                       "\r\n<br />"
                       "<table><tr><th align='left'>SoftAP config</th></tr>"
                       "<tr><td>SSID ")) +
            String(AP_SSID_NAME) +
            F("</td></tr>"
              "<tr><td>IP ") +
            _toStringIp(WiFi.softAPIP()) +
            F("</td></tr>"
              "</table>"
              "\r\n<br />"
              "<table><tr><th align='left'>WLAN config</th></tr>"
              "<tr><td>SSID ") +
            String(This->_eprom._cur_ssid_name) +
            F("</td></tr>"
              "<tr><td>IP ") +
            _toStringIp(WiFi.localIP()) +
            F("</td></tr><tr><td>Time Zone:")+
            String(This->_eprom._offset)+
            F("</td></tr></table>"
              "\r\n<br />"
              "<table><tr><th align='left'>WLAN list (if any missing predd F5)</th></tr>");
    Serial.println("scan start");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) +
                    ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") +
                    WiFi.RSSI(i) + F(")</td></tr>");
        }
    } else {
        Page += F("<tr><td>No WLAN found</td></tr>");
    }
    Page += F(
                "</table>"
                "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
                "<input type='text' placeholder='SSID' name='n'/>"
                "<br /><input type='text' placeholder='PASSWORD' name='p'/>"
                "<br /><input type='text' placeholder='192.168.1.178' name='i'/>"
                "<br /><input type='text' placeholder='Time Zone in hours' name='z'/>"
                "<br /><input type='submit' value='Apply'/></form>");
    Page += This->_end_html();

    ESP_S()->send(200, "text/html", Page);
    ESP_S()->client().stop(); // Stop is needed because we sent no content length
}

void esp_32_base_c::handleWifiSave()
{
    TRACE();
    This->_otaing=false;
    Serial.println("wifi save");
    ESP_S()->arg("n").toCharArray(This->_eprom._cur_ssid_name, sizeof(This->_eprom._cur_ssid_name) - 1);
    ESP_S()->arg("p").toCharArray(This->_eprom._cur_ssid_pswd, sizeof(This->_eprom._cur_ssid_pswd) - 1);
    ESP_S()->arg("i").toCharArray(This->_eprom._ipstatic, sizeof(This->_eprom._ipstatic) - 1);
    ESP_S()->arg("z").toCharArray(This->_eprom._offset, sizeof(This->_eprom._offset) - 1);

    String page = This->_start_html();
    page += "SAVING & REBOOTING";
    page += This->_end_html();
    ESP_S()->send(200, "text/plain", page);

    ESP_S()->client().stop(); // Stop is needed because we sent no content length
    This->_saveCredentials();
    REBOOT();
    This->_b_conn2wifi = This->_eprom.sig==SIG; // Request WLAN _b_conn2wifi with new credentials if there is a SSID

}

void esp_32_base_c::handleNotFound()
{
    TRACE();
    This->_otaing=false;
    if (This->_captivePortal()) { // If caprive portal redirect instead of displaying the error page.
        return;
    }
    String message =This->_start_html();
    message += F("File Not Found\n\n");
    message += F("URI: ");
    message += ESP_S()->uri();
    message += F("\nMethod: ");
    message += (ESP_S()->method() == HTTP_GET) ? "GET" : "POST";
    message += F("\nArguments: ");
    message += ESP_S()->args();
    message += F("\n");

    for (uint8_t i = 0; i < ESP_S()->args(); i++) {
        message += String(F(" ")) + ESP_S()->argName(i) + F(": ") + ESP_S()->arg(i) + F("\n");
    }

    message += This->_end_html();
    ESP_S()->send(404, "text/plain", message);

}


void esp_32_base_c::handleRoot()
{
    if (This->_captivePortal())
    {
        return;
    }
    if(This->_bsta)
    {
        esp_32_base_c::handleWifi();
    }
    else
    {
        String page;
        This->page_request(ESP_S(),page);
        ESP_S()->send(200, "text/html", page);
    }
}

void esp_32_base_c::otaUpdate()
{
    TRACE();
    HTTPUpload& upload = ESP_S()->upload();

    Serial.printf("Status = %d", upload.status);

    if (upload.status == UPLOAD_FILE_START)
    {
        TRACE();
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(900000)) { //start with max available size
            TRACE();
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_WRITE)
    {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
            TRACE();
            Update.printError(Serial);
        }
    } else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true)) { //true to set the size to the current progress
            Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            Serial.flush();
            //ESP.restart();
        } else {
            Update.printError(Serial);
        }
    }
    TRACE();
}
