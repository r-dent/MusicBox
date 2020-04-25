
#ifndef CONFIG_SERVER_H_
#define CONFIG_SERVER_H_

#include "Arduino.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <FS.h>
#include <SD.h>
#include <SDCardHelper.h>

class ConfigServer {

public:
    void setup();
    void loop();
    String RFIDCardUid = "";
    char const *ssid = "music.box";
    char const *password = "12345678";
    char const *serverName = "musicbox";

private:
    AsyncWebServer webServer = AsyncWebServer(80);
    File uploadFile; 
    SDCardHelper sdCard;
    unsigned long t_start = 0;
    unsigned long t_stop = 0;

    void homePage(AsyncWebServerRequest *request);
    void fileDownload(AsyncWebServerRequest *request); // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
    void fileUpload(AsyncWebServerRequest *request);
    void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void sendHTMLHeader(AsyncResponseStream *response);
    void selectInput(String heading1, String command, String arg_calling_name, AsyncResponseStream *response);
    void appendPageHeader(AsyncResponseStream *response);
    void appendPageFooter(AsyncResponseStream *response);
    String fileSize(int bytes);
    double calcSpeed(unsigned long ms, size_t len);

};

#endif