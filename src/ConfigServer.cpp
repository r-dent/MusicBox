#include "ConfigServer.h"

void ConfigServer::setup()
{

    // Connect to Wi-Fi network with SSID and password
    Serial.print("Setting AP (Access Point)…");
    // Remove the password parameter, if you want the AP (Access Point) to be open
    WiFi.softAP(ssid);

    IPAddress ipAddress = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(ipAddress);

    if (!MDNS.begin(serverName))
    {
        Serial.println("Problem starting DNS.");
    }

    webServer.onFileUpload([this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        this->handleUpload(request, filename, index, data, len, final);
    });
    webServer.on("/", [this](AsyncWebServerRequest *request) { this->homePage(request); });
    webServer.on("/download", [this](AsyncWebServerRequest *request) { this->fileDownload(request); });
    webServer.on("/upload", [this](AsyncWebServerRequest *request) { this->fileUpload(request); });
    webServer.onNotFound([](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not found"); });
    webServer.begin();

    Serial.println("HTTP server started");
}

void ConfigServer::loop()
{
}

void ConfigServer::homePage(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    sendHTMLHeader(response);

    response->print("<a href='/download'><button>Download</button></a>");
    response->print("<a href='/upload'><button>Upload</button></a>");
    response->print("<ul>");
    sdCard.handleFiles("/", [response](File entry) {
        const char *fileName = entry.name();
        response->printf("<li>%s</li>", fileName);
    });
    response->print("</ul>");

    appendPageFooter(response);

    request->send(response);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ConfigServer::fileDownload(AsyncWebServerRequest *request)
{
    // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
    if (request->args() > 0)
    { // Arguments were received
        if (request->hasArg("download"))
            request->send(SD, request->arg("download"));
    }
    else
    {
        AsyncResponseStream *response = request->beginResponseStream("text/html");
        selectInput("Enter filename to download", "download", "download", response);
        request->send(response);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ConfigServer::fileUpload(AsyncWebServerRequest *request)
{
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    appendPageHeader(response);
    response->print("<h3>Select File to Upload</h3>");
    response->print("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
    response->print("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
    response->print("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
    response->print("<a href='/'>[Back]</a><br><br>");
    appendPageFooter(response);
    request->send(response);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void ConfigServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{

    if (!index)
    {
        // String filename = upload.filename;
        if (!filename.startsWith("/"))
            filename = "/" + filename;
        Serial.printf("UploadStart: %s\n", filename.c_str());

        if (SD.exists(filename))
            SD.remove(filename); // Remove a previous version, otherwise data is appended the file again
        
        t_start = millis();
        uploadFile = SD.open(filename, FILE_WRITE); // Open the file for writing in SPIFFS (create it, if doesn't exist)
        filename = String();
    }
    if (uploadFile)
        uploadFile.write(data, len);
    if (final)
    {
        t_stop = millis();
        Serial.print("Time UPLOAD: "); Serial.print((t_stop - t_start) / 1000.0); Serial.println(" sec.");
        Serial.print("Speed UPLOAD: "); Serial.print(calcSpeed(t_stop - t_start, index + len)); Serial.println(" Kbit/s");
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
        uploadFile.close();
        fileUpload(request);
    }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ConfigServer::sendHTMLHeader(AsyncResponseStream *response)
{
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    appendPageHeader(response);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ConfigServer::selectInput(String heading1, String command, String arg_calling_name, AsyncResponseStream *response)
{
    sendHTMLHeader(response);
    response->printf("<h3>%s</h3>", heading1.c_str());
    response->printf("<FORM action='/%s' method='post'>", command.c_str());
    response->printf("<input type='text' name='%s' value=''><br>", arg_calling_name.c_str());
    response->printf("<input type='submit' name='%s' value=''><br>", arg_calling_name.c_str());
    response->print("<a href='/'>[Back]</a>");
    appendPageFooter(response);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String ConfigServer::fileSize(int bytes)
{
    String fsize = "";
    if (bytes < 1024)
        fsize = String(bytes) + " B";
    else if (bytes < (1024 * 1024))
        fsize = String(bytes / 1024.0, 3) + " KB";
    else if (bytes < (1024 * 1024 * 1024))
        fsize = String(bytes / 1024.0 / 1024.0, 3) + " MB";
    else
        fsize = String(bytes / 1024.0 / 1024.0 / 1024.0, 3) + " GB";
    return fsize;
}

void ConfigServer::appendPageHeader(AsyncResponseStream *response)
{
    response->print("<!DOCTYPE html><html>");
    response->print("<head>");
    response->print("<title>File Server</title>"); // NOTE: 1em = 16px
    response->print("<meta name='viewport' content='user-scalable=yes,initial-scale=1.0,width=device-width'>");
    response->print("<style>");
    response->print(F("body{max-width:65%;margin:0 auto;font-family:arial;font-size:105%;text-align:center;color:blue;background-color:#F7F2Fd;}"));
    response->print("ul{list-style-type:none;margin:0.1em;padding:0;border-radius:0.375em;overflow:hidden;background-color:#dcade6;font-size:1em;}");
    // response->print("li{float:left;border-radius:0.375em;border-right:0.06em solid #bbb;}last-child {border-right:none;font-size:85%}");
    // response->print("li a{display: block;border-radius:0.375em;padding:0.44em 0.44em;text-decoration:none;font-size:85%}");
    response->print("li a:hover{background-color:#EAE3EA;border-radius:0.375em;font-size:85%}");
    response->print("section {font-size:0.88em;}");
    response->print("h1{color:white;border-radius:0.5em;font-size:1em;padding:0.2em 0.2em;background:#558ED5;}");
    response->print("h2{color:orange;font-size:1.0em;}");
    response->print("h3{font-size:0.8em;}");
    response->print("table{font-family:arial,sans-serif;font-size:0.9em;border-collapse:collapse;width:85%;}");
    response->print("th,td {border:0.06em solid #dddddd;text-align:left;padding:0.3em;border-bottom:0.06em solid #dddddd;}");
    response->print("tr:nth-child(odd) {background-color:#eeeeee;}");
    response->print(F(".rcorners_n {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:75%;}"));
    response->print(F(".rcorners_m {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:50%;color:white;font-size:75%;}"));
    response->print(F(".rcorners_w {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:70%;color:white;font-size:75%;}"));
    response->print(F(".column{float:left;width:50%;height:45%;}"));
    response->print(".row:after{content:'';display:table;clear:both;}");
    response->print("*{box-sizing:border-box;}");
    response->print("footer{background-color:#eedfff; text-align:center;padding:0.3em 0.3em;border-radius:0.375em;font-size:60%;}");
    response->print(F("button{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:20%;color:white;font-size:130%;}"));
    response->print(F(".buttons {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:80%;}"));
    response->print(".buttonsm{border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:9%; color:white;font-size:70%;}");
    response->print(F(".buttonm {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:15%;color:white;font-size:70%;}"));
    response->print(F(".buttonw {border-radius:0.5em;background:#558ED5;padding:0.3em 0.3em;width:40%;color:white;font-size:70%;}"));
    response->print("a{font-size:75%;}");
    response->print("p{font-size:75%;}");
    response->print("</style></head><body><h1>File Server </h1>");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ConfigServer::appendPageFooter(AsyncResponseStream *response)
{ // Saves repeating many lines of code for HTML page footers
    response->print("<ul>");
    response->print("<li><a href='/'>Home</a></li>"); // Lower Menu bar command entries
    response->print("<li><a href='/download'>Download</a></li>");
    response->print("<li><a href='/upload'>Upload</a></li>");
    response->print("</ul>");
    response->print("</body></html>");
}

double ConfigServer::calcSpeed(unsigned long ms, size_t len){
    return (double)(len * 125) / (double)(ms * 16);
}
