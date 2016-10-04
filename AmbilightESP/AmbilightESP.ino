
#include <ESP8266WiFi.h>
#include <NeoPixelBus.h>

// Ardulight:
//  FF 0C 15 29 58 6B 95 2F 3F 62
//  FF 52 68 95 5C 6C 93 29 37 58
//  FF 52 68 95 5C 6C 93 29 39 59

// Adalight
//             [1][2][3][4][5][6][7][8][9][.][.][.][.][.][.]
// one   led:  41 64 61 00 00 55 44 4B 58
// two   leds: 41 64 61 00 01 54 44 4B 58 0A 0A 0A
// three leds: 41 64 61 00 02 54 44 4B 58 0A 0A 0A 0A 0A 0A
// [1]   = 41 - ??
// [2]   = 64 - ??
// [3]   = 61 - ??
// [4]   = highByte of leds count 
// [5]   = lowByte of leds count  (0 is 1 led, 1 is 2 leds,  etc...)
// [6]   = Red value of led[0]
// [7]   = Green value of led[0]
// [8]   = Blue value of led[0]
// [9]   = Red value of led[1]
// [10]  = Green value of led[1]
// [11]  = Blue value of led[1]
// [...] = etc for leds count


const uint16_t PixelCount = 25;


#ifdef ESP8266
  #define USE_TCP
  #define USE_SERIAL Serial
  const char* ssid = "NPMGroup-Guest";
  const char* password = "";
  NeoPixelBus<NeoRgbFeature, NeoEsp8266BitBang800KbpsMethod> strip(PixelCount, 13);
  #ifdef USE_TCP
    WiFiServer server(23);
    WiFiClient serverClients[1];
  #endif
#else
  #define USE_SERIAL Serial
  NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, A5);
#endif

unsigned int leds = 0;
String inputString = "";

void setup() {
  strip.Begin();
  strip.Show();
  #ifdef ESP8266
    WiFi.begin(ssid);
    while(WiFi.status() != WL_CONNECTED) {delay(500);}
  #endif
  #ifdef USE_SERIAL
    USE_SERIAL.begin(115200);
    #ifdef ESP8266 && USE_TCP
      USE_SERIAL.print("Ready! Use 'telnet ");
      USE_SERIAL.print(WiFi.localIP());
      USE_SERIAL.println(" 23' to connect");
    #endif
  #endif
  #ifdef USE_TCP
    server.begin();
    server.setNoDelay(true);
  #endif
}

void loop() {
  #ifdef USE_SERIAL
    serialEvent();
  #endif
  #ifdef USE_TCP
    tcpEvent();
  #endif
}

#ifdef USE_TCP
void tcpEvent() {
  if(server.hasClient()) {
    //find free/disconnected spot
    if(!serverClients[0] || !serverClients[0].connected()){
      if(serverClients[0]) serverClients[0].stop();
      serverClients[0] = server.available();
    }
    //no free/disconnected spot so reject
    WiFiClient serverClient = server.available();
    serverClient.stop();
  }
  //check clients for data
  if(serverClients[0] && serverClients[0].connected()) {
    if(serverClients[0].available()) {
      //get data from the telnet client and push it to the UART
      if(serverClients[0].available()) {
        if(!inputString.endsWith("Ada")) {
          inputString += (char) serverClients[0].read();
        } else {
          leds = serverClients[0].read();
          leds = leds << 8;
          while(!serverClients[0].available());
          leds |= serverClients[0].read();
          for(byte n=0; n<=leds; n++) {
            byte RGB[3];
            for(byte i=0;i<3;i++) {
              while(!serverClients[0].available());
              RGB[i] = serverClients[0].read();
            }
            RgbColor color(RGB[2], RGB[1], RGB[0]);
            strip.SetPixelColor(n, color);
          }
          RgbColor color(0, 0, 0);
          for(byte n=leds+1; n<PixelCount; n++) {
            strip.SetPixelColor(n, color);
          }
          strip.Show();
          inputString = "";
        }
      }
    }
  }
}
#endif

#ifdef USE_SERIAL
void serialEvent() {
  if(USE_SERIAL.available()) {
    if(!inputString.endsWith("Ada")) {
      inputString += (char) USE_SERIAL.read();
    } else {
      leds = USE_SERIAL.read();
      leds = leds << 8;
      while(!USE_SERIAL.available());
      leds |= USE_SERIAL.read();
      for(byte n=0; n<=leds; n++) {
        byte RGB[3];
        for(byte i=0;i<3;i++) {
          while(!USE_SERIAL.available());
          RGB[i] = USE_SERIAL.read();
        }
        RgbColor color(RGB[2], RGB[1], RGB[0]);
        strip.SetPixelColor(n, color);
      }
      RgbColor color(0, 0, 0);
      for(byte n=leds+1; n<PixelCount; n++) {
        strip.SetPixelColor(n, color);
      }
      strip.Show();
      inputString = "";
    }
  }
}
#endif
