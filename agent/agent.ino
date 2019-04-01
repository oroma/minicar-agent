#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif

void drawGraph();

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const int motor = 12;
const int led = 13;

void handleRoot() {
  digitalWrite(led, 1);
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void handleConfig() {
  if (server.hasArg("plain")== false) {
    server.send(200, "text/plain", "Body not received");
    return;
  }

  JSONVar myObject = JSON.parse(server.arg("plain"));
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object

  // { "anglePin": 12, "speedPin": 13 }
  if (myObject.hasOwnProperty("anglePin")) {
    Serial.print("myObject[\"anglePin\"] = ");

    Serial.println((int) myObject["anglePin"]);
  }

  if (myObject.hasOwnProperty("speedPin")) {
    Serial.print("myObject[\"speedPin\"] = ");

    Serial.println((int) myObject["speedPin"]);
  }
  
  String message = "Body received:\n";
         message += server.arg("plain");
         message += "\n";
  String res  = "{";
         res += "\"result\": 0,";
         res += "\"request\": \"config\",";
         res += "\"time\": ";
         res += millis();
         res += "}";

  server.send(200, "text/plain", res);
  Serial.println(message);
}

void handleCommand() {
  if (server.hasArg("plain")== false) {
    server.send(200, "text/plain", "Body not received");
    return;
  }

  JSONVar myObject = JSON.parse(server.arg("plain"));
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }
  Serial.print("JSON.typeof(myObject) = ");
  Serial.println(JSON.typeof(myObject)); // prints: object

  // Extract values
  // { "sensor": "sensor field", "time": 123456789, "data": [0.1, 0.2, 0.3, 0.4] }
  if (myObject.hasOwnProperty("sensor")) {
    Serial.print("myObject[\"sensor\"] = ");

    Serial.println((const char*)myObject["sensor"]);
  }

  if (myObject.hasOwnProperty("time")) {
    Serial.print("myObject[\"time\"] = ");

    Serial.println((int) myObject["time"]);
  }

  if (myObject.hasOwnProperty("data")) {
    JSONVar myArray = JSON.parse(JSON.stringify(myObject["data"]));
    Serial.print("myObject[\"data\"] = ");
    for (int i = 0; i < myArray.length(); i++) {
      JSONVar value = myArray[i];
      // Serial.println(JSON.typeof(value));
      Serial.print(value);
      if (i < (myArray.length() - 1))
        Serial.print(", ");
    }
  }
  Serial.println();
  
  String message = "Body received:\n";
         message += server.arg("plain");
         message += "\n";
  String res  = "{";
         res += "\"result\": 0,";
         res += "\"request\": \"command\",";
         res += "\"time\": ";
         res += millis();
         res += "}";

  server.send(200, "text/plain", res);
  Serial.println(message);
}

void handleStatus() {
}

void setup(void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/config", handleConfig);
  server.on("/command", handleCommand);
  server.on("/status", handleStatus);

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}
