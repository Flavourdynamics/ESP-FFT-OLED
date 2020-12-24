#ifndef OTA_h
#define OTA_h

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFiMulti.h>

const char* WIFIhost = "example.com";
const char* WIFIssid0 = "BYRNE-2.4";
const char* WIFIpassword0 = "Keigan123";
const char* WIFIssid1 = "MSO";
const char* WIFIpassword1 = "eightchar";
//const char* WIFIip = "192.168.0.30";
IPAddress WIFIip(192, 168, 0, 100);   
IPAddress WIFIgateway(192, 168, 31, 1);
IPAddress WIFIsubnet(255, 255, 0, 0);

WiFiMulti WIFImulti;
WebServer WIFIserver(80);

/////////// Login page //////////////////
const char* WIFIloginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Password' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";

//////////// Server Index Page //////////////////
const char* WIFIserverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

void OTAsetup(){
  // Connect to WiFi network
  WiFi.config(WIFIip, WIFIgateway, WIFIsubnet);  // WiFi.config(ip, dns, gateway, subnet); 
  WIFImulti.addAP(WIFIssid0, WIFIpassword0 );
  WIFImulti.addAP(WIFIssid1, WIFIpassword0 );
  Serial.println("");
  
  Serial.println("Connecting Wifi...");
  // Wait for connection
  while (WIFImulti.run() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /*use mdns for host name resolution*/
  if (!MDNS.begin(WIFIhost)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  WIFIserver.on("/", HTTP_GET, []() {
    WIFIserver.sendHeader("Connection", "close");
    WIFIserver.send(200, "text/html", WIFIloginIndex);
  });
  WIFIserver.on("/serverIndex", HTTP_GET, []() {
    WIFIserver.sendHeader("Connection", "close");
    WIFIserver.send(200, "text/html", WIFIserverIndex);
  });
  /*handling uploading firmware file */
  WIFIserver.on("/update", HTTP_POST, []() {
    WIFIserver.sendHeader("Connection", "close");
    WIFIserver.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = WIFIserver.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
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
  WIFIserver.begin();
}

#endif
