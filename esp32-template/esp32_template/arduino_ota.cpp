
#include <ESP8266WebServer.h>

#include "arduino_ota.h"


const char* serverIndex =
        "<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
        "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
        "</form>";

arduino_ota::arduino_ota()
{

}

void arduino_ota::setup(ESP8266WebServer* pws)
{
//    pws->on("/", HTTP_GET, [&]() {
//        pws->sendHeader("Connection", "close");
//        pws->send(200, "text/html", loginIndex);
//    });
    pws->on("/serverindex", HTTP_GET, [&]() {
        pws->sendHeader("Connection", "close");
        pws->send(200, "text/html", serverIndex);
    });
    /*handling uploading firmware file */
    pws->on("/update", HTTP_POST, [&]() {
        pws->sendHeader("Connection", "close");
        pws->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
    }, [&]() {
        HTTPUpload& upload = pws->upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update: %s\n", upload.filename.c_str());
            if (!Update.begin(0xFFFFFFFF)) { //start with max available size
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            /* flashing firmware to ESP*/
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) { //true to set the size to the current progress
                Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });
}

void arduino_ota::loop(ESP8266WebServer* pws)
{
    pws->handleClient();
    delay(1);
}
