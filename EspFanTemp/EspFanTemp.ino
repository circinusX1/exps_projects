/*
 * Sketch: ESP8266_LED_Control_06_Station_Mode_with_mDNS_and_wifiManager
 * Control an LED from a web browser
 * Intended to be run on an ESP8266
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

#include <ESP8266WiFi.h>
//#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

// change these values to match your network
char ssid[] = "marius1";       //  your network SSID (name)
char pass[] = "*************";    // your network password

WiFiServer server(80);

String header = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
String html_1 = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='18; URL='/'' /><meta name='viewport' content='width=device-width, initial-scale=1.0'/><meta charset='utf-8'><style>body {font-size:140%;} #main {display: table; margin: auto;  padding: 0 10px 0 10px; } h2,{text-align:center; } .button { padding:10px 10px 10px 10px; width:100%;  background-color: #4CAF50; font-size: 120%;}</style><title>FAN SERTAR</title></head><body><div id='main'><h2>SERTAR FAN</h2>";
String html_2 = "";
String html_4 = "</div></body></html>";

String request = "";
int LED_Pin = 14;

#define BMP085_ADDRESS 0x77
// Adafruit_BME280 bme;

//#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

#define FAN_ON  LOW
#define FAN_OFF HIGH

void scan(void);
float read_eprom();
void write_eprom(float temp);
static float temp_limit=40.0;
void setup()
{
    EEPROM.begin(512);
    pinMode(LED_Pin, OUTPUT);
    pinMode(2, OUTPUT);
    digitalWrite(LED_Pin, 0);

    Wire.begin(4,5);
    Serial.begin(115200);
    delay(500);
    Serial.println(F("Serial started at 115200"));
    Serial.println();
    scan();


    // We start by connecting to a WiFi network
    Serial.print(F("Connecting to "));  Serial.println(ssid);
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");    delay(500);
    }
    Serial.println("");
    Serial.println(F("[CONNECTED]"));
    Serial.print("[IP ");
    Serial.print(WiFi.localIP());
    Serial.println("]");

    // WiFiManager
    WiFiManager wifiManager;
    IPAddress _ip = IPAddress(192,168,1, 229);
    IPAddress _gw = IPAddress(192,168,1, 1);
    IPAddress _sn = IPAddress(255, 255, 255, 0);
    //end-block2

    wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);

    wifiManager.autoConnect("marius1","zoomahia1");


    // or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    // if you get here you have connected to the WiFi
    Serial.println("Connected.");


    if (!MDNS.begin("esp8266"))   {  Serial.println("Error setting up MDNS responder!");  }
    else                          {  Serial.println("mDNS responder started");  }
    server.begin();
    Serial.println("Server started");


    // int status = bme.begin(0x77);
    // if (!status) {
    //  Serial.println("Could not find a valid BME280 sensor, check wiring!");
    //  while (1);
    // }//

    if(!bmp.begin())
    {
        /* There was a problem detecting the BMP085 ... check your connections */
        Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
        while(1);
    }

    temp_limit=read_eprom();
    write_eprom(temp_limit);




} // void setup()

typedef struct thp{
    float temp;
    float pres;
    float alt;

}thp;

void printValues(thp* t) ;
static int k=0;

static thp t={0,0,0};
int ledstate=0;

char out[32];
int TOGLE=0;

void loop()
{

    // Check if a client has connected
    WiFiClient client = server.available();


    if (client)
    {
        // Read the first line of the request
        request = client.readStringUntil('\r');

        Serial.println("request:");
        Serial.println(request);

        if    ( request.indexOf("LEDON") > 0 )  {
            digitalWrite(LED_Pin, FAN_ON);
            ledstate=1;
        }
        else if  ( request.indexOf("LEDOFF") > 0 ) {
            digitalWrite(LED_Pin, FAN_OFF);
            ledstate=0;
        }
        else if  ( request.indexOf("TEMP") > 0 ) {
            int index = request.indexOf("=");
            if(index>0)
            {
                String val =request.substring(index+1);

                temp_limit = val.toFloat();
                write_eprom(temp_limit);
                Serial.println(val);
            }

        }


        // Get the LED pin status and create the LED status message
        if (digitalRead(LED_Pin) == FAN_ON)
        {
            // the LED is on so the button needs to say turn it off
            html_2 = "<form id='F1' action='LEDOFF'><input class='button' style='background-color: #3F3;' type='submit' value='TURN OFF FAN' ></form><hr>";
            ledstate=1;
        }
        else
        {
            // the LED is off so the button needs to say turn it on
            html_2 = "<form id='F1' action='LEDON'><input class='button' style='background-color: #666;' type='submit' value='TURN ON FAN' ></form><hr>";
            ledstate=0;
        }
        temp_limit = read_eprom();
        html_2.concat("<form action='/' id='F2' method='GET' ><input type='text' name='TEMP' value='");
        sprintf(out,"%2.3f",temp_limit);
        html_2.concat(out);
        html_2.concat("'><input class='button' type='submit' value='SETTEMP' ></form><br>");
        html_2.concat("<div style='font-size:2em';border:1px solid black;>");

        if(t.temp){
            sprintf(out,"%2.3fC",t.temp);
            html_2.concat("<li>Temp:");html_2.concat(out);
            sprintf(out,"%2.3fHPa",t.pres);
            html_2.concat("<li>Pres:");html_2.concat(out);
            sprintf(out,"%2.3fm",t.alt);
            html_2.concat("<li>Alt:");html_2.concat(out);
            sprintf(out,"%2.3fC",temp_limit);
            html_2.concat("<li><font color='red'>Trigger:");html_2.concat(out);
        }
        html_2.concat("</font></div>");

        client.flush();

        client.print( header );
        client.print( html_1 );
        client.print( html_2 );
        client.print( html_4);

        delay(5);

        // The client will actually be disconnected when the function returns and 'client' object is detroyed

    }
    if(k++%20==0){
        t.temp=0;
        printValues(&t);
        if(t.temp>0)
        {
            if(t.temp >= temp_limit+1.0f)
            {
                ledstate=1;
                digitalWrite(LED_Pin, FAN_ON);
                Serial.print("FAN ON");
            }
            else if(t.temp < temp_limit-1.0f)
            {
                digitalWrite(LED_Pin, FAN_OFF);
                ledstate=0;
                Serial.print("FAN OFF");
            }
        }
    }
    if(k%5==0)
        digitalWrite(2, TOGLE=!TOGLE);
    delay(100);

} // void loop()


void printValues(thp* t)
{

    sensors_event_t event;
    bmp.getEvent(&event);
    if (event.pressure)
    {
        /* Display atmospheric pressue in hPa */
        //Serial.print("Pressure:    ");
        //Serial.print(event.pressure);
        t->pres=event.pressure;
        //Serial.println(" hPa");
        bmp.getTemperature(&t->temp);
        //Serial.print("Temperature: ");
        //Serial.print(t->temp);
        //Serial.println(" C");
        float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;
        //Serial.print("Altitude:    ");
        t->alt=bmp.pressureToAltitude(seaLevelPressure,
                                      event.pressure);
        //Serial.print(t->alt);
        //Serial.println(" m");
        //Serial.println("");
    }
}


void scan(void)
{
    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address<16)
                Serial.print("0");
            Serial.print(address,HEX);
            Serial.println("  !");

            nDevices++;
        }
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
    delay(3000);

}


float read_eprom(void)
{
    String strText;
    char byte= EEPROM.read(0x0F);
    if(byte!=0x5A)
    {
        EEPROM.write(0x0f,0x5A);
        return 40.0;
    }
    for(int i=1;i<16;i++)
    {
        strText = strText + char(EEPROM.read(0x0F+i));
    }
    return strText.toFloat();
}


void write_eprom(float temp)
{
    char eprom[16]={0};

    EEPROM.write(0x0f,0x5A);
    sprintf(eprom,"%2.3f",temp);
    for(int i=1;i<16;i++)
    {
        EEPROM.write(0x0F+i, eprom[i-1]);
    }
}
