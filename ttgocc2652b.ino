#include <ESPmDNS.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

#define TFT_GREY 0x5AEB // New colour
#define TRIGGER_PIN 35
#define BACKLIGHT_PIN 0
#define BAUDS_PIN 21
#define RAZCC_PIN 33
int timeout = 120; // seconds to run for
#define MAX_SRV_CLIENTS 1
int j ='2';
int bl ='1';
AsyncWebServer serverMDNS(80);
WiFiServer server(8888);
WiFiClient serverClients[MAX_SRV_CLIENTS];

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h

void setup() { 
  pinMode(TRIGGER_PIN, INPUT);
  pinMode(BAUDS_PIN, INPUT_PULLUP);
  pinMode(BACKLIGHT_PIN, INPUT);
  pinMode(RAZCC_PIN, OUTPUT);
  digitalWrite(RAZCC_PIN, LOW);
  tft.init();
  tft.setRotation(1);
  
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_RED);  tft.setTextFont(2);tft.setTextSize (2);
  tft.println("CC2652P2 & TTGO IP Server Telnet");
  tft.setTextSize (1);
  tft.setTextColor(TFT_YELLOW); //tft.setTextFont(1);
  tft.println("For KoenKK's Z2M");
  tft.setTextColor(TFT_RED,TFT_BLACK); //   tft.setTextFont(1);
  tft.print("11/05/2021    Nivek  ");   
  
  if(digitalRead(BAUDS_PIN) == LOW ){
  Serial.begin(57600);
  Serial2.begin(57600, SERIAL_8N1, 25, 26);//ttgo
  //Serial2.begin(57600);
     Serial.println("serial2 57600");
  tft.setTextColor(TFT_GREEN); //   tft.setTextFont(1);
  tft.println("57600 Bauds"); // Should print DEADBEEF
  }else{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 25, 26);//ttgo
  //Serial2.begin(115200);
     Serial.println("serial2 115200");
  tft.setTextColor(TFT_GREEN); //   tft.setTextFont(1);
  tft.println("115200 Bauds"); // Should print DEADBEEF
  }
  
  Serial.println("\nConnecting");
    
WiFi.mode(WIFI_STA);
initWifiManager();
  
    Serial.println("");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  tft.setTextColor(TFT_BLUE); //   tft.setTextFont(1);
  tft.print("port : 8888    IP : "); // Should print DEADBEEF
  tft.println(WiFi.localIP()); // Should print DEADBEEF


if(!MDNS.begin("serialGateway")) {
     Serial.println("Error starting mDNS");
     return;
}
MDNS.addService("z-stack", "tcp", 80);
serverMDNS.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Hello World");
  });
   
  serverMDNS.begin();


  //start UART and the server
  server.begin();
  server.setNoDelay(true);

  Serial.print("Ready! Use 'telnet ");
  Serial.print(WiFi.localIP());
  Serial.println(" 8888' to connect");
  Serial.println("attente client");
}

void loop() {
    handleButton();
  uint8_t i;
  
    if (server.hasClient()){
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        //find free/disconnected spot
        if (!serverClients[i] || !serverClients[i].connected()){
          if(serverClients[i]) serverClients[i].stop();
          serverClients[i] = server.available();
          if (!serverClients[i]) Serial.println("available broken");
          Serial.print("New client: ");
          j = '1';
          Serial.print(i); Serial.print(' ');
          Serial.println(serverClients[i].remoteIP());
              tft.init();
              tft.setRotation(1);
              tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
            tft.setTextColor(TFT_RED);  tft.setTextFont(2);tft.setTextSize (1);
            tft.print("new client  ");
            tft.print(serverClients[i].remoteIP());
          digitalWrite(RAZCC_PIN, HIGH);
          break;
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        //no free/disconnected spot so reject
        server.available().stop();
      }
    }
    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        if(serverClients[i].available()){
            tft.setTextColor(TFT_BLUE,TFT_BLACK);
            tft.setCursor(0, 16);
            //tft.print("tx -- ");
          Serial.println(" ");
          Serial.print(" tx -- ");
          //get data from the telnet client and push it to the UART
          while(serverClients[i].available()) {
            
            char c = serverClients[i].read();
            Serial2.write(c);
              if (c <= 0x0F){
                Serial.print("0");
                tft.print("0");
              }
            Serial.print(c, HEX);
            Serial.print(" ");
            tft.print(c, HEX);
            tft.print(" ");
          }
            tft.println("                                                                                                                                                                          ");
          /*Serial.print("rx -- ");
          while(serverClients[i].available()) Serial.write(serverClients[i].read());
              tft.init();
              tft.setRotation(1);
              tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
            tft.setTextColor(TFT_RED);  tft.setTextFont(2);tft.setTextSize (1);
            tft.println("new client");
            tft.println(serverClients[i].remoteIP());
            tft.setTextColor(TFT_BLUE);
            tft.setCursor(0, 30, 2);
            tft.println("rx --");
          while(serverClients[i].available()) tft.println(serverClients[i].read());*/
        }
      }
      else {
        if( j == '1'){
          Serial.println("deconnexion client");
              tft.init();
              tft.setRotation(1);
              tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
            tft.setTextColor(TFT_RED);  tft.setTextFont(2);tft.setTextSize (1);
            tft.println("client disconnect");
            digitalWrite(RAZCC_PIN, LOW);
            j = '0';
        }
        if (serverClients[i]) {
          serverClients[i].stop();
        }
      }
    }
    //check UART for data
    if(Serial2.available()){
      size_t len = Serial2.available();
      uint8_t sbuf[len];
      Serial2.readBytes(sbuf, len);
      //push UART data to all connected telnet clients
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        if (serverClients[i] && serverClients[i].connected()){
          serverClients[i].write(sbuf, len);
          Serial.println(" ");
          Serial.print(" rx -- ");
         //Serial.write(sbuf, len);
            /*  tft.init();
              tft.setRotation(1);
              tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
            tft.setTextColor(TFT_RED);  tft.setTextFont(2);tft.setTextSize (1);
            tft.println("new client");
            tft.println(serverClients[i].remoteIP());*/
            tft.setTextColor(TFT_YELLOW,TFT_BLACK); //tft.setTextFont(1);
            tft.setCursor(0, 63, 2);
            //tft.print("rx -- ");
            for (int i = 0; i<len; ++i) {
                  if ((char) sbuf[i] <= 0x0F){
                    Serial.print("0");
                    tft.print("0");
                  }
                tft.print((char) sbuf[i], HEX);
                tft.print(" ");
                Serial.print((char) sbuf[i], HEX);
                Serial.print(" ");
            }
            /*String sendBuf;
            for (int i = 0; i<len; ++i) {
                sendBuf += (char) sbuf[i];
            }
            tft.println(sendBuf);*/
            tft.println("                                                                                                                                                                          ");
          //delay(1);
        }
      }
    }

}

void handleButton(){
  int debounce = 200;
  if ( digitalRead(TRIGGER_PIN) == LOW ){
    delay(debounce);
    if(digitalRead(TRIGGER_PIN) == LOW ){        
              tft.init();
              tft.setRotation(1);
              tft.fillScreen(TFT_BLACK);
            tft.setCursor(0, 0, 2);
            tft.setTextColor(TFT_GREEN);  tft.setTextFont(2);tft.setTextSize (1);
            tft.println("Mode AP reglage wifi");
      WiFiManager wifiManager;    
      wifiManager.resetSettings();
      delay(1000);
      ESP.restart();
    }
  }

  if(digitalRead(BACKLIGHT_PIN) == LOW ){
    delay(debounce);
      if(digitalRead(BACKLIGHT_PIN) == LOW ){
          if(bl == '1'){
            bl = '0';
            digitalWrite(TFT_BL, 0);
        //Serial.println("ecran eteint");
          }else{
            bl = '1';
            digitalWrite(TFT_BL, 1);
        //Serial.println("ecran marche");
          }
      }
  }
}  

void initWifiManager(){
WiFiManager wifiManager;
wifiManager.setDebugOutput(true);
  wifiManager.setCustomHeadElement("<style>body{background: #000000;}button{transition: 0.3s;opacity: 0.8;cursor: pointer;border:0;border-radius:1rem;background-color:#de0b00;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}button:hover {opacity: 1}button[type=\"submit\"]{margin-top: 15px;margin-bottom: 10px;font-weight: bold;text-transform: capitalize;}input{height: 30px;font-family:verdana;margin-top: 5px;background-color: rgb(253, 253, 253);border: 0px;-webkit-box-shadow: 2px 2px 5px 0px rgba(0,0,0,0.75);-moz-box-shadow: 2px 2px 5px 0px rgba(0,0,0,0.75);box-shadow: 2px 2px 5px 0px rgba(0,0,0,0.75);}div{color: #a71414;}div a{text-decoration: none;color: #a10000;}div[style*=\"text-align:left;\"]{color: black;}, div[class*=\"c\"]{border: 0px;}a[href*=\"wifi\"]{border: 2px solid #a71414;text-decoration: none;color: #a10000;padding: 10px 30px 10px 30px;font-family: verdana;font-weight: bolder;transition: 0.3s;border-radius: 5rem;}a[href*=\"wifi\"]:hover{background: #a10000;color: white;}</style>");
  //wifiManager.setMinimumSignalQuality(40);
IPAddress _ip = IPAddress(192, 168, 4, 25);
IPAddress _gw = IPAddress(192, 168, 4, 1);
IPAddress _sn = IPAddress(255, 255, 255, 0);
wifiManager.setAPStaticIPConfig(_ip, _gw, _sn);


bool res;
  // res = wm.autoConnect(); // auto generated AP name from chipid
   res = wifiManager.autoConnect("cc2652_Gateway"); // anonymous ap
  //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect or hit timeout");
  tft.setTextColor(TFT_YELLOW); //tft.setTextFont(1);
  tft.println("Mode AP WB 192.168.4.25");
    // ESP.restart();
  } 
  else {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
  tft.setTextColor(TFT_YELLOW); //tft.setTextFont(1);
  tft.println("Mode Clients Wifi Connecte");
  }
}
