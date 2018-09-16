#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define D0 16
#define D1 5 // I2C Bus SCL (clock)
#define D2 4 // I2C Bus SDA (data)
#define D3 0
#define D4 2 // Same as "LED_BUILTIN", but inverted logic
#define D5 14 // SPI Bus SCK (clock)
#define D6 12 // SPI Bus MISO 
#define D7 13 // SPI Bus MOSI
#define D8 15 // SPI Bus SS (CS)
#define D9 3 // RX0 (Serial console)
#define D10 1 // TX0 (Serial console)

#define STATE_ON 0
#define STATE_OFF 1

#define STATE_OFF 1

const char* ssid     = "giay123";
const char* password = "123giay123";
const char* host = "167.99.66.144";

void setup() {
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  digitalWrite(D0, STATE_OFF);
  digitalWrite(D1, STATE_OFF);
  digitalWrite(D2, STATE_OFF);
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password); //works!

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(10000);
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/state/HELLO/1";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
               delay(10);

 unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  // Example Response Data: {"data":[{"pin":"pin0","state":"off"},{"pin":"pin1","state":"off"}],"status":"successful"}
  String res = "{}";
  while (client.available() || client.connected()) {
    res = client.readStringUntil('\r');
    Serial.println("->" + res);
  }
  Serial.println("-> Response Data: " + res);
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(res);
  delay(10);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  JsonArray& data = root["data"].asArray();
  for (int i = 0; i < data.size(); i++) {
    String pinName = data[i]["pin"];
    String pinState = data[i]["state"];
    Serial.println("pinName: " + pinName);
    Serial.println("pinState: " + pinState);
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
