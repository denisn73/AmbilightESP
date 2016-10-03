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

#define USE_SERIAL Serial

const uint16_t PixelCount = 4;
const uint8_t  PixelPin = 2;

NeoPixelBus<NeoGrbFeature, NeoEsp8266BitBang800KbpsMethod> strip(PixelCount, PixelPin);

void setup() {
  #ifdef USE_SERIAL
    USE_SERIAL.begin(115200);
  #else
  #endif
  strip.Begin();
  strip.Show();
}

void loop() {
  serialEvent();
}

#ifdef USE_SERIAL
bool new_string = false;
bool getHighLeds = false;
bool getLowLeds = false;
unsigned int leds = 0;
unsigned int ledIndex = 0;
String inputString = "";
void serialEvent() {
  while(USE_SERIAL.available()) {
    uint8_t cmd = USE_SERIAL.read();
    if(!new_string) {
      inputString += (char)cmd;
      if(inputString.endsWith("Ada")) {
        inputString = "";
        getHighLeds = false;
        getLowLeds  = false;
        new_string  = true;
        ledIndex    = 0;
        leds        = 0;
      }
    } else if(!getHighLeds) {
      leds = cmd;
      leds = leds << 8;
      getHighLeds = true;
    } else if(!getLowLeds) {
      leds |= cmd;
      getLowLeds = true;
    } else {
      byte R = USE_SERIAL.read();
      byte G = USE_SERIAL.read();
      byte B = USE_SERIAL.read();
      if(ledIndex < PixelCount) {
        RgbColor RGB(R, G, B);
        strip.SetPixelColor(ledIndex, RGB);
      }
      if(ledIndex!=leds) ledIndex++;
      else {
        strip.Show();
        new_string = false;
      }
    }
  }
}
#else
void serialEvent(...);
#endif
