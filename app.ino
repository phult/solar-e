#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

// digital PIN - GPIO map
#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2  // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3  // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

// mode configurations
#define STATE_ON 0
#define STATE_OFF 1
#define DEBUG_MODE false

// WIFI configurations
const char *WIFI_SSID = "";
const char *WIFI_PASSWORD = "";

// service configurations
const char *SERVICE_HOST = "167.99.66.144";
const int SERVICE_PORT = 80;
const String SERVICE_ROUTE = "/state";
const int POOLING_INTERVAL = 10000;
const String API_KEY = "";
const String DEVICE_ID = "";

// available pins
const int PIN_SZIE = 3;
int pins[PIN_SZIE] = { D0, D1, D2 };

void setup() {
  // pins  
  for (int i = 0; i < PIN_SZIE; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], STATE_OFF);
  }  
  pinMode(D4, OUTPUT);
  digitalWrite(D4, 1);
  log("WiFi connected - IP address: " + WiFi.localIP());
  // serial
  Serial.begin(115200);
  delay(10);
}

void loop() {
  delay(POOLING_INTERVAL);
  if (checkOrEstablishWiFiConnection(WIFI_SSID, WIFI_PASSWORD)) {
    String res = sendRequest(SERVICE_HOST, SERVICE_PORT, SERVICE_ROUTE + "/" + API_KEY + "/" + DEVICE_ID);
    log("received response data: " + res);
    parseResponseData(res); 
  }  
}

bool checkOrEstablishWiFiConnection(const char *ssid, const char *password) {
  bool retval = true;
  if (WiFi.status() != WL_CONNECTED) {
    inProcess();
    log("connecting to the WiFi network: ", true);
    log(ssid);    
    WiFi.begin(ssid, password);
    for (int i = 0; i < 20; i++) {
      if ( WiFi.status() != WL_CONNECTED ) {        
        delay(500);
        inProcess();
        retval = false;
      } else {
        retval = true;
        break;
      }
    }
  }
  return retval;
}

void parseResponseData(String res) {
  // Example Response Data: {"data":[{"pin":"pin0","state":"off"},{"pin":"pin1","state":"off"}],"status":"successful"}  
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(res);
  delay(10);
  // Check if json parsing succeeds.
  if (!root.success() && root["data"] == NULL) {
    log("parse response data failed");
    return;
  }
  // Parsing
  JsonArray &data = root["data"].asArray();
  for (int i = 0; i < data.size(); i++) {
    String pinName = data[i]["pin"];
    String pinState = data[i]["state"];
    log("pinName: " + pinName);
    log("pinState: " + pinState);
    int pin = D0;
    int state = LOW;
    if (pinName == "pin0") {
      pin = D0;
    } else if (pinName == "pin1") {
      pin = D1;
    } else if (pinName == "pin2") {
      pin = D2;
    }
    if (pinState == "on") {
      state = STATE_ON;
    } else if (pinState == "off") {
      state = STATE_OFF;
    }
    // write pin state
    digitalWrite(pin, state);
    delay(10);
  }
}

String sendRequest(String host,int port, String route) {
  String res = "{}";
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  if (!client.connect(host, port)) {
    log("cannot connect to host");
    return res;
  }
  // Send the request to the server
  client.print(String("GET ") + route + " HTTP/1.1\r\n" + "Host: " + host +
               "\r\n" + "Connection: close\r\n\r\n");
  delay(10);
  // Waiting for the server to respond
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      log("request client timeout!");
      client.stop();
      return res;
    }
  }
  // Read all the lines of the reply from server
  while (client.available() || client.connected()) {
    res = client.readStringUntil('\r');
    log("->" + res);
  }
  // return last response line
  return res;
}

void log(String logString) {
  log(logString, false);
}

void log(String logString, bool isInline) {
  if (!DEBUG_MODE) {
    return;
  }
  if (isInline != NULL && isInline) {
    Serial.print(logString);
  } else {
    Serial.println(logString);
  }
}

void inProcess() {    
  log(".", true);
  digitalWrite(D4, 0);  
  delay(100);
  digitalWrite(D4, 1);
  delay(100);
}