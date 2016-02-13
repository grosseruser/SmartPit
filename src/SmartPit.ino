
// *************** include
// TFT
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
// WEB
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
// sonstiges
#include <pgmspace.h>
#include <math.h>

// *************** define, vars, init

// MCP3208 (soft SPI)
#define SCK   14
#define MISO  12
#define MOSI  13
#define CS    16

// TFT Display
#define TFT_DC 15
#define TFT_CS 2
#define TFT_MOSI 13
#define TFT_CLK 14
#define TFT_RST 0
#define TFT_MISO 12

// Use hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

uint32_t nextrun=0;
uint8_t actdis=0;
int16_t esum=0;
float temps[8];

const PROGMEM int16_t d_pos[] = {
  20,9,
  20,49,
  180,9,
  180,49,
  20,89,
  20,129,
  180,89,
  180,129
};

// WEB 
const char* ssid = "***";
const char* password = "***";

ESP8266WebServer server(80);

// *************** function
void soft_spi_init(){ // MCP3208
  pinMode(MOSI, OUTPUT);
  pinMode(SCK,  OUTPUT);
  pinMode(CS,   OUTPUT);
  pinMode(MISO, INPUT);
  
  digitalWrite(MOSI, HIGH);
  digitalWrite(SCK,  HIGH);
  digitalWrite(CS,   HIGH);
  digitalWrite(MISO, LOW);
}
 
long softspi(int adr){ // MCP3208
  long erg=0;
  adr |= 0b00011000;
  digitalWrite(CS,   LOW);
  
  for (uint8_t i=7; i; i--){
    digitalWrite(SCK,   LOW);
    if (adr & 0b00010000) 
      digitalWrite(MOSI, HIGH);
    else 
      digitalWrite(MOSI, LOW);
    adr = adr << 1;
    delayMicroseconds(1);
    digitalWrite(SCK,   HIGH);
    delayMicroseconds(1);
  }
  // read 12bit 
  for (uint8_t i=12; i; i--){
    digitalWrite(SCK,   LOW);
    erg = erg <<1;
    if (digitalRead(MISO))  erg |= 0x01; 
    delayMicroseconds(1);
    digitalWrite(SCK,   HIGH);
    delayMicroseconds(1);
  }
    
  digitalWrite(CS,   HIGH);
  return erg;
}

float calcT (uint32_t r, uint32_t typ){ // MCP3208-TFT (messwrumrechnung)
  float Rmess = 0;
  float a = 0.003339; // IKEA Fantast
  float b = 0.000251911;
  float c = 0.00000351094;
  float Rv = 46.7;
  float R25 = 47;
  float v = 0;
  float erg = 0;
  
  if (typ==1){ // Outdoor Chef 
    R25 = 1100;
    a = 0.003365;
    b = 0.000231;
    c = 0.000004;
  }else if (typ==2){ // ?
    a = 0.003339;
    b = 0.000251911;
    c = 0.00000351094;
  }
  
  Rmess = Rv*((4096.0/(4096-r)) - 1);
  v = log(Rmess/R25);
  erg = (1/(a + b*v + c*v*v)) - 273;
  return (erg>-30)?erg:0;

}

void screen1_base(){ // TFT
  tft.fillScreen(ILI9341_BLACK);

  tft.setTextColor(ILI9341_YELLOW); tft.setTextSize(1);
  tft.setCursor(20, 0);
  tft.print("Fleisch links:");
  tft.setCursor(20, 40);
  tft.println("Gahrraum links:");
  tft.setCursor(180, 0);
  tft.print("Fleisch rechts:");
  tft.setCursor(180, 40);
  tft.print("Gahrraum rechts:");
  tft.setCursor(20, 80);
  tft.print("Sensor 5:");
  tft.setCursor(20, 120);
  tft.println("Sensor 6:");
  tft.setCursor(180, 80);
  tft.print("Sensor 7:");
  tft.setCursor(180, 120);
  tft.print("Sensor 8:");
  
  /*
  for(int16_t y=80; y<240; y+=32) tft.drawFastHLine(0,  y, 319, tft.color565(32,32,32));
  for(int16_t x=0;  x<320; x+=32) tft.drawFastVLine(x, 80, 159, tft.color565(32,32,32));//0x39E7
  tft.drawRect(0, 80, 320, 160, ILI9341_YELLOW);
  */
}

void ausg(uint16_t nr, uint16_t pox, uint16_t poy, float T){ // TFT/seriell
  
    /*Serial.print("x: ");
    Serial.print(pox);
    Serial.print(" y: ");
    Serial.println(poy);*/
    Serial.print("Temperatur ");
    Serial.print(nr);
    Serial.print(" :");
    Serial.println(T);
    
    tft.setCursor((int16_t) pox, (int16_t) poy);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(3);
    tft.print(T);
    tft.print("C ");
}

void tft_data(uint16_t x, uint16_t y, String data){ // TFT
  tft.setCursor((int16_t) x, (int16_t) y);
  tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  tft.setTextSize(3);
  tft.print(data);
  tft.print("  ");
}

int16_t limit(int16_t a, int16_t min, int16_t max){ return (a>max)?max:(a<min)?min:a;}

int16_t PID_Regler (float x, float w){ // pitmaster    
  float e = w - x;                  
  esum = esum + e*5;      
  esum=limit(esum,-1023,1023);
  float y = (e*50)+esum; // PI-Regler
  return limit((int16_t) y, 0, 1023);
}

void handleRoot() { // WEB
  String Response;
  Response  = F("<!DOCTYPE html><html><head><title>WLAN Thermometer Micro</title>\n");
  Response += F("<meta charset=\"utf-8\">\n");
  //Response += F("<meta http-equiv=\"refresh\" content=\"5\" >\n");
  Response += F("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">\n");
  Response += F("</head><body style=\"background-color:#CCCCCC;\">\n");
  Response += F("<span id=\"table\"></span>\n");
  //Response += F("<br><br>Debug: <span id=\"debu\" ></span>\n");
  Response += F("<script type=\"text/javascript\">\n");
  Response += F("var xmlHttp = new XMLHttpRequest();\n");
  Response += F("function $(id){return document.getElementById(id);}\n");
  Response += F("function ajax_send(data){\n");
  Response += F(" xmlHttp.open('POST', '/ajax.txt', true);\n");
  Response += F(" xmlHttp.setRequestHeader('Content-type','application/x-www-form-urlencoded');\n");
  Response += F(" xmlHttp.onreadystatechange = function () {if (xmlHttp.readyState==4 && xmlHttp.status==200) {ajax_parse(eval(xmlHttp.responseText));};}\n");
  //Response += F(" $(\"debu\").innerHTML=('ajax=true'+data);\n");
  Response += F(" xmlHttp.send('ajax=true'+data);\n");
  Response += F("}\n");
  Response += F("function ajax_parse(data){\n");
  Response += F(" //document.getElementById('ajax').innerHTML=data;\n");
  Response += F(" for(var i = 0; i < 8; i++) $(\"s\"+i).innerHTML=data[i+2];\n");
  //Response += F(" $(\"rgaug\").innerHTML=data[2];\n");
  Response += F("}\n");
  Response += F("function tic(){\n");
  Response += F(" ajax_send();\n");
  Response += F(" window.setTimeout(\"tic();\",3200);\n");
  Response += F("}\n");
  Response += F("var tmp=\"<table width='100%'><tr>\";\n");
  Response += F("for(var i = 0; i < 4; i++){\n");
  Response += F("  tmp += \"<td></td><td style='width:200px; font-size:12px;'>Sensor\"+ (i*2+1) +\"</td>\";\n");
  Response += F("  tmp += \"<td style='width:200px; font-size:12px;'>Sensor\"+ (i*2+2) +\"</td><td></td></tr>\";\n");
  Response += F("  tmp+=\"<tr><td></td><td style='font-size:32px;'><span id='s\"+ (i*2) +\"' ></span>&deg;C</td>\";\n");
  Response += F("  tmp+=\"<td style='font-size:32px;'><span id='s\"+ (i*2+1) +\"' ></span>&deg;C</td><td></td></tr>\";\n");
  Response += F("}\n");
  Response += F("$(\"table\").outerHTML = tmp+\"</table>\";\n");
  Response += F("tic();\n");
  Response += F("</script>\n");
  Response += F("</body></html>\n");
  server.send(200, "text/html", Response);
}

void handleNotFound(){ // WEB
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleAJAX() { // WEB
  String Response;
  if(server.hasArg("dings")) {
    Serial.println("DINGS! @");
  }
  Response = "{["+String((millis()/1000))+","+(uint32_t) ESP.getFreeHeap();
  for(int8_t i=0; i<8; i++) Response += "," + String(temps[i],1);
  Response += "]}\n";
  server.send(200, "text/html", Response);
}

void WiFiStart() { // WEB
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
}

// *************** Setup
void setup(void) {
  Serial.begin(115200);
  soft_spi_init();
  tft.begin();
  tft.setRotation(3);
  screen1_base();

  // Webserver
  WiFiStart();
  if (MDNS.begin("thermo")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.on("/ajax.txt", handleAJAX);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  tft_data(20, 169, WiFi.localIP().toString());
  // Pitmaster
  //pinMode(4, OUTPUT);
  //analogWrite(4, 1023);
}

// *************** main
void loop() {
  if (WiFi.status() != WL_CONNECTED){ WiFiStart(); }
  server.handleClient();

  if (millis()>=nextrun){
    nextrun=millis()+250;
    Serial.println("------\n");
    
    soft_spi_init();// HW SPI ausschalten
    for(int8_t i=0; i<8; i++) temps[i] = calcT(softspi(i),((i>3)?0:1));
    SPI.begin();// HW SPI einschalten

    uint16_t x = (uint16_t)pgm_read_word(&d_pos[(actdis*2)]);
    uint16_t y = (uint16_t)pgm_read_word(&d_pos[(actdis*2)+1]);
    
    ausg(actdis+1,x,y,temps[actdis]);

    
    /*
      // Pitmaster  
      uint16_t ry = (uint16_t) PID_Regler(T[0], 51.0);
      Serial.print("Regler y: ");
      Serial.println(ry);
      analogWrite(4, 1023-ry);
      tft_data(20, 169, float(ry));
    */
    
    if (++actdis>7) actdis = 0;
  }
    
}
