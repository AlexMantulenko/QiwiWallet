#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define SP D6

#define OLED_RESET LED_BUILTIN
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* host = "edge.qiwi.com";
const int httpsPort = 443;  // HTTPS= 443 and HTTP = 80
const char fingerprint[] PROGMEM = "6D 07 61 9B 15 9B 60 88 93 FD 2E 54 04 99 C6 62 77 5D 0F B1"; // SHA-1

String lineUI;

int i = 60;
int j = 0;

float balance = 0;
float newBalance = 0;
float goalPrice = 1300;

const char* ssid = "******";
const char* password = "******";


void setup() {
  Serial.begin(115200);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32 
    for(;;);
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("Connecting to "));
  display.print(ssid);
  display.display();
  delay(3000);

  display.clearDisplay();
  display.setCursor(0,0);
  display.print("Connecting");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
   delay(500);
   display.setCursor(i,j);
   display.print(".");
   i = i + 5;
   if(i == 125) {
    i = 0;
    j = 10;
   }
   if(i == 125 && j == 10) {
    display.clearDisplay();
    display.setCursor(10,14);
    display.println(F("Connection failed!"));
    display.display();
    delay(2000);
    break;
   }
   display.display();
  }

  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("WiFi connected"));
  display.println(F("IP address: "));
  display.println(WiFi.localIP());
  display.display();
}

void loop() {
  GetUserInfo();
  parseBalance();
  delay(5000);
  showGoal();
  delay(3000);
}

void GetUserInfo() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  String link = "/funding-sources/v2/persons/phoneNumber/accounts";
  WiFiClientSecure httpsClient;
  httpsClient.setFingerprint(fingerprint);
  httpsClient.setTimeout(15000);
  delay(1000);

  Serial.print("HTTPS Connecting");
  int r = 0;
  while((!httpsClient.connect(host, httpsPort)) && (r < 30)) {
      delay(100);
      Serial.print(".");
      r++;
  }
  if(r == 30) {
    display.setCursor(10, 14);
    display.println(F("Connection failed!"));
    display.display();
  }
  else { }
  delay(1000);
  display.clearDisplay();

  httpsClient.print(String("GET ") + link + " HTTP/1.1\r\n" +
                    "Content-Type: application/json\r\n" +
                    "Accept: application/json\r\n" +
                    "Authorization: Bearer *QiwiApiKey*\r\n" +
                    "Host: " + host + "\r\n" +
                    "Connection: close\r\n\r\n");

  while (httpsClient.connected()) {
    String str = httpsClient.readStringUntil('\n');
    if (str == "\r") {
      break;
    }
  }
  while(httpsClient.available()) {
    lineUI = httpsClient.readStringUntil('\n');
    Serial.println(lineUI);
  }
}


void parseBalance() {
  const size_t bufferSize1 = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(5) + 140;
  DynamicJsonBuffer jsonBuffer1(bufferSize1);
  JsonObject& rootUI = jsonBuffer1.parseObject(lineUI);

  display.clearDisplay();
  display.setFont();
  balance = newBalance;
  newBalance = rootUI["accounts"][0]["balance"]["amount"];
  if(((newBalance > balance) || (newBalance < balance)) && balance != 0) {
    tone(SP, 1000, 1000);
  }
  display.setCursor(0, 0);
  display.println(F("Your balance: "));
  display.println(F(""));
  display.setTextSize(1);
  display.setFont(&FreeMono9pt7b);
  display.print(newBalance);
  display.println(F("RUB"));
  display.display();
}

void showGoal() {
  display.clearDisplay();
  display.setFont();
  display.setCursor(0, 0);
  display.println(F("Goal: Some goal"));
  display.println(F(""));
  display.print(F("Left: "));
  display.print(goalPrice - newBalance);
  display.print(F(" RUB"));

  display.display();

}
