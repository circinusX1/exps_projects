
#include "ap_wifi_ota.h"
//#include <ArduinoOTA.h>

const char*     AP_SSID_NAME = "HVAC";
const char*     AP_SSID_PASS = "HVACHVAC";
const char*     HOST_NAME = "HVAC-1.0";
const byte      DNS_PORT = 53;
const int       LedUsb = 16;
const int       LedAnt = 2;
static esp32_full*  This;
#define ESP_S()     This->_esp_srv

esp32_full::esp32_full(byte a, byte b, byte c, byte d, int port)
{
    This = this;
    _ip[0] = a;
    _ip[0] = b;
    _ip[0] = c;
    _ip[0] = d;
    _dns_srv = new DNSServer;
    _esp_srv = new ESP8266WebServer(80);
    _ip_addr = new IPAddress(10, 5, 5, 1);
    _net_mask = new IPAddress(255, 255, 0, 0);
}

esp32_full::~esp32_full()
{
    delete _dns_srv ;
    delete ESP_S() ;
    delete _ip_addr ;
    delete _net_mask;
}

bool    esp32_full::setup()
{
      delay(100);
    Serial.begin(115200);
    Serial.println("");
    Serial.println("Configuring access point...");
    
    //pinMode(LedUsb, OUTPUT);
    pinMode(LedAnt, OUTPUT);
    //digitalWrite(LedUsb, HIGH);
    digitalWrite(LedAnt, HIGH);

    Serial.println("Configuring access point...");
    
    WiFi.softAPConfig(*_ip_addr, *_ip_addr, *_net_mask);
    WiFi.softAP(AP_SSID_NAME, AP_SSID_PASS);
    delay(800);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.flush();
    
    _dns_srv->setErrorReplyCode(DNSReplyCode::NoError);
    _dns_srv->start(DNS_PORT, "*", *_ip_addr);

    ESP_S()->on("/", esp32_full::handleRoot);
    ESP_S()->on("/index.html", esp32_full::handleRoot);
    ESP_S()->on("/wifi", esp32_full::handleWifi);
    ESP_S()->on("/ota", esp32_full::handleOta);
    ESP_S()->on("/wifisave", esp32_full::handleWifiSave);
    ESP_S()->on("/generate_204", esp32_full::handleRoot);   //  Android captive portal. Maybe not needed. Might be handled by notFound handler.
    ESP_S()->on("/fwlink", esp32_full::handleRoot);         //  Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
    ESP_S()->onNotFound(esp32_full::handleNotFound);
    ESP_S()->begin();
    Serial.println("HTTP ESP_S() started");
    _loadCredentials();
    _b_connect = strlen(_cur_ssid_name) > 0;
#ifdef _ARDUINOOTA_H_
    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

    ArduinoOTA.begin();
#endif 
    digitalWrite(LedUsb, LOW);
    digitalWrite(LedAnt,LOW);
    Serial.flush();
    return true;
}

bool    esp32_full::loop()
{
    ++_loop;
    if(_loop % _blink==0){
        digitalWrite(LedUsb, _toggle);
        digitalWrite(LedAnt, _toggle);
        _toggle = !_toggle;
    }

    if(_otaing){
#ifdef _ARDUINOOTA_H_      
        ArduinoOTA.handle();
#endif         
        _blink = 10000;
        return true;
    }
    _blink = 10;
    if (_b_connect) {
        Serial.println("Connect requested");
        _b_connect = false;
        _connectWifi();
        _last_conn = millis();
    }
    do{
        unsigned int s = WiFi.status();
        if (s == 0 && millis() > (_last_conn + 60000)) {
            _b_connect = true;
        }
        if (_wlan_status != s) { // WLAN _wlan_status change
            Serial.print("Status: ");
            Serial.println(s);
            _wlan_status = s;
            if (s == WL_CONNECTED) {
                Serial.println("");
                Serial.print("Connected to ");
                Serial.println(_cur_ssid_name);
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());

                if (!MDNS.begin(HOST_NAME)) {
                    Serial.println("Error setting up MDNS responder!");
                } else {
                    Serial.println("mDNS responder started");
                    MDNS.addService("http", "tcp", 80);
                }
            } else if (s == WL_NO_SSID_AVAIL) {
                WiFi.disconnect();
            }
        }
        if (s == WL_CONNECTED) {
            MDNS.update();
        }
    }while(0);

    _dns_srv->processNextRequest();
    ESP_S()->handleClient();
    this->user_loop(_loop);
    delay (10);
    return true;
}

void esp32_full::_loadCredentials()
{
    EEPROM.begin(512);
    EEPROM.get(0, _cur_ssid_name);
    EEPROM.get(0 + sizeof(_cur_ssid_name), _cur_ssid_pswd);
    char ok[2 + 1];
    EEPROM.get(0 + sizeof(_cur_ssid_name) + sizeof(_cur_ssid_pswd), ok);
    EEPROM.end();
    if (String(ok) != String("OK")) {
        _cur_ssid_name[0] = 0;
        _cur_ssid_pswd[0] = 0;
    }
    Serial.println("Recovered credentials:");
    Serial.println(_cur_ssid_name);
    Serial.println(strlen(_cur_ssid_pswd) > 0 ? "********" : "<no _cur_ssid_pswd>");
}

void esp32_full::_connectWifi()
{
    Serial.println("Connecting as wifi client...");
    WiFi.disconnect();
    WiFi.begin(_cur_ssid_name, _cur_ssid_pswd);
    IPAddress _ip = IPAddress(_ip[0],_ip[1],_ip[2],_ip[3]);
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

void esp32_full::_saveCredentials()
{
    EEPROM.begin(512);
    EEPROM.put(0, _cur_ssid_name);
    EEPROM.put(0 + sizeof(_cur_ssid_name), _cur_ssid_pswd);
    char ok[2 + 1] = "OK";
    EEPROM.put(0 + sizeof(_cur_ssid_name) + sizeof(_cur_ssid_pswd), ok);
    EEPROM.commit();
    EEPROM.end();
}

boolean esp32_full::_isIp(String str)
{
    for (size_t i = 0; i < str.length(); i++) {
        int c = str.charAt(i);
        if (c != '.' && (c < '0' || c > '9')) {
            return false;
        }
    }
    return true;
}

String esp32_full::_toStringIp(IPAddress ip)
{
    String res = "";
    for (int i = 0; i < 3; i++) {
        res += String((ip >> (8 * i)) & 0xFF) + ".";
    }
    res += String(((ip >> 8 * 3)) & 0xFF);
    return res;
}

#define NOP() asm volatile ("nop")
void IRAM_ATTR esp32_full::delayMicroseconds2(uint32_t us)
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

boolean esp32_full::_captivePortal()
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

void esp32_full::handleOta()
{
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");
    ESP_S()->send(200, "text/html", "OTA ON");
    This->_otaing=true;
}

void esp32_full::handleRoot()
{
    This->_otaing=false;
    if (This->_captivePortal())
    {
        return;
    }
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");

    String page;
     This->page_request("GET","/",page);

    ESP_S()->send(200, "text/html", page);
}

void esp32_full::handleWifi()
{

    This->_otaing=false;
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");

    String Page;
    Page += F(
                "<!DOCTYPE html><html lang='en'><head>"
                "<meta name='viewport' content='width=device-width'>"
                "<title>poweramp</title></head><body><h4>"
                "<h1>Wifi config</h1>");
    if (ESP_S()->client().localIP() == *This->_ip_addr) {
        Page += String(F("<p>Connected toSoft AP: ")) + AP_SSID_NAME + F("</p>");
    } else {
        Page += String(F("<p>Connected to Wifi network: ")) + This->_cur_ssid_name + F("</p>");
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
            String(This->_cur_ssid_name) +
            F("</td></tr>"
              "<tr><td>IP ") +
            _toStringIp(WiFi.localIP()) +
            F("</td></tr>"
              "</table>"
              "\r\n<br />"
              "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
    Serial.println("scan start");
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n > 0) {
        for (int i = 0; i < n; i++) {
            Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? F(" ") : F(" *")) + F(" (") +
                    WiFi.RSSI(i) + F(")</td></tr>");
        }
    } else {
        Page += F("<tr><td>No WLAN found</td></tr>");
    }
    Page += F(
                "</table>"
                "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
                "<input type='text' placeholder='network' name='n'/>"
                "<br /><input type='_cur_ssid_pswd' placeholder='_cur_ssid_pswd' name='p'/>"
                "<br /><input type='submit' value='Connect/Disconnect'/></form>"
                "<p>You may want to <a href='/'>return to the home page</a>.</p>"
                "</h4></body></html>");
    ESP_S()->send(200, "text/html", Page);
    ESP_S()->client().stop(); // Stop is needed because we sent no content length
}

void esp32_full::handleWifiSave()
{
    This->_otaing=false;
    Serial.println("wifi save");
    ESP_S()->arg("n").toCharArray(This->_cur_ssid_name, sizeof(This->_cur_ssid_name) - 1);
    ESP_S()->arg("p").toCharArray(This->_cur_ssid_pswd, sizeof(This->_cur_ssid_pswd) - 1);
    ESP_S()->sendHeader("Location", "wifi", true);
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");
    ESP_S()->send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
    ESP_S()->client().stop(); // Stop is needed because we sent no content length
    This->_saveCredentials();
    This->_b_connect = strlen(This->_cur_ssid_name) > 0; // Request WLAN _b_connect with new credentials if there is a SSID

}

void esp32_full::handleNotFound()
{
    This->_otaing=false;
    if (This->_captivePortal()) { // If caprive portal redirect instead of displaying the error page.
        return;
    }
    String message = F("File Not Found\n\n");
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
    ESP_S()->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    ESP_S()->sendHeader("Pragma", "no-cache");
    ESP_S()->sendHeader("Expires", "-1");
    ESP_S()->send(404, "text/plain", message);
}
