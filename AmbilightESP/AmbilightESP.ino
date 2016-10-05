
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
  //#define USE_TCP
  #define USE_FAST_TCP
  #define USE_SERIAL Serial
  const char* ssid = "NPMGroup-Guest";
  const char* password = "";
  NeoPixelBus<NeoRgbFeature, NeoEsp8266BitBang800KbpsMethod> strip(PixelCount, 13);
  #ifdef USE_TCP || USE_FAST_TCP
    
    //WiFiClient serverClients[1];
  #endif
#else
  NeoPixelBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount, A5);
#endif
#define USE_SERIAL Serial
WiFiServer server(23);
unsigned int leds = 0;
String inputString = "";

void setup() {
  strip.Begin();
  strip.Show();
  #ifdef ESP8266
    WiFi.begin(ssid);
    while(WiFi.status() != WL_CONNECTED) {delay(500);}
    server.begin();
    server.setNoDelay(true);
  #endif
  #ifdef USE_SERIAL
    USE_SERIAL.begin(115200);
    USE_SERIAL.print("Ready! Use 'telnet ");
    USE_SERIAL.print(WiFi.localIP());
    USE_SERIAL.println(" 23' to connect");
  #endif
}

void loop() {
  if(USE_SERIAL.available()) dataEncode(USE_SERIAL.read());
  //serialEvent();
  #ifdef ESP8266
  if(server.available()) dataEncode(server.read());
  //tcpEvent();
  #endif
}

char test[3];
bool testAda(byte data) {
  test[0] = test[1];
  test[1] = test[2];
  test[2] = (char)data;
  if(test[0]=='A'&&test[0]=='d'&&test[0]=='a') return true;
  return false;
}

byte rgb_buf[3];
bool getAda = false;
byte getSize = 0;
unsigned int buf_size = 0;
unsigned int buf_index = 0;
unsigned int rgb_index = 0;
void dataEncode(byte data) {
  if(!getAda) {
    if(testAda(data)) getAda = true;
  } else if(getSize<2) {
    if(getSize==0) {
      buf_size = data;
    } else {
      buf_size = buf_size << 8;
      buf_size |= data;
      buf_size = (buf_size+1)*3;
    }
    getSize++;
  } else if(buf_index<buf_size) {
    rgb_buf[rgb_index] = data;
    rgb_index++;
    if(rgb_index>=3) {
      rgb_index = 0;
      RgbColor color(rgb_buf[2], rgb_buf[1], rgb_buf[0]);
      strip.SetPixelColor(buf_index/3, color);
    }
    buf_index++;
  } else {
    if((char)data=='A') strip.Show();
    getAda = false;
    testAda(data);
  }  
}

//#ifdef USE_TCP
//void tcpEvent() {
//  if(server.hasClient()) {
//    //find free/disconnected spot
//    if(!serverClients[0] || !serverClients[0].connected()){
//      if(serverClients[0]) serverClients[0].stop();
//      serverClients[0] = server.available();
//    }
//    //no free/disconnected spot so reject
//    WiFiClient serverClient = server.available();
//    serverClient.stop();
//  }
//  //check clients for data
//  if(serverClients[0] && serverClients[0].connected()) {
//    if(serverClients[0].available()) {
//      //get data from the telnet client and push it to the UART
//      if(serverClients[0].available()) {
//        if(!inputString.endsWith("Ada")) {
//          inputString += (char) serverClients[0].read();
//        } else {
//          leds = serverClients[0].read();
//          leds = leds << 8;
//          while(!serverClients[0].available());
//          leds |= serverClients[0].read();
//          for(byte n=0; n<=leds; n++) {
//            byte RGB[3];
//            for(byte i=0;i<3;i++) {
//              while(!serverClients[0].available());
//              RGB[i] = serverClients[0].read();
//            }
//            RgbColor color(RGB[2], RGB[1], RGB[0]);
//            strip.SetPixelColor(n, color);
//          }
//          RgbColor color(0, 0, 0);
//          for(byte n=leds+1; n<PixelCount; n++) {
//            strip.SetPixelColor(n, color);
//          }
//          strip.Show();
//          inputString = "";
//        }
//      }
//    }
//  }
//}
//#endif

//#ifdef USE_SERIAL
//void serialEvent() {
//  if(USE_SERIAL.available()) {
//    if(!inputString.endsWith("Ada")) {
//      inputString += (char) USE_SERIAL.read();
//    } else {
//      leds = USE_SERIAL.read();
//      leds = leds << 8;
//      while(!USE_SERIAL.available());
//      leds |= USE_SERIAL.read();
//      for(byte n=0; n<=leds; n++) {
//        byte RGB[3];
//        for(byte i=0;i<3;i++) {
//          while(!USE_SERIAL.available());
//          RGB[i] = USE_SERIAL.read();
//        }
//        RgbColor color(RGB[2], RGB[1], RGB[0]);
//        strip.SetPixelColor(n, color);
//      }
//      RgbColor color(0, 0, 0);
//      for(byte n=leds+1; n<PixelCount; n++) {
//        strip.SetPixelColor(n, color);
//      }
//      strip.Show();
//      inputString = "";
//    }
//  }
//}
//#endif
