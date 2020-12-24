#include <FastLED.h>
#define LEDtilewidth 4 // How much width each input controls
#define LEDtileheight 60 // how high each strip is
#define LEDtilehorz 7 // number of matrices arranged horizontally
#define LEDtilevert 1  // how many tiles stacked vertically
#define LEDstrips (LEDtilewidth*LEDtilehorz)
#define LEDper (LEDtileheight*LEDtilevert)
#define LEDtotal (LEDstrips*LEDper)
#include "EQ.h"
#include "OTA.h"
#include <BluetoothSerial.h>
#include "heltec.h"

BluetoothSerial Bluetooth;
TaskHandle_t Core0;

CRGB leds[LEDtotal];
byte hue;

int loopval = 18;

void setup() {
  Serial.begin(250000);
  FastLED.setCorrection(TypicalSMD5050);
  FastLED.setBrightness(10);
  //FastLED.setMaxRefreshRate(180);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, 2000);
  FastLED.clear();
  
  pinMode(ledPin, OUTPUT);
  EQsampletimer = round(1000000 * (1.0 / EQsamplefreq));
  Bluetooth.begin("Starshroud");
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
  
  Heltec.display->setContrast(255);
  Heltec.display->setLogBuffer(5, 30);
  Heltec.display->clear();

  xTaskCreatePinnedToCore(Core0fn, "Core0", 10000, NULL, 1, &Core0, 0);
  OTAsetup();
  
  delay(1000);
}

void Core0fn( void * pvParameters ){
  while(1){   
    WIFIserver.handleClient();
    EVERY_N_MILLIS(1000){
      if(WIFImulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
      }
    }
    vTaskDelay(1);   
  }
}

void loop() { 
  barz();
  //textbuffer();
  
  //EVERY_N_SECONDS_I(shufloop, STATEshuffleinterval){
  //EVERY_N_MILLIS_I(looper, loopval){     
    EQget();
    EQstats();
    EQnoisegate();   
    EQbeatDetection();    
    EQbeatBuckets();
    EQbeatBlink();
    
    //EQprintone(7);
    //EQprintall();
    //EQprintallmaxes();
  //}
  //justtext();

  //EVERY_N_MILLIS(8){
   // quadplexor();
    //visualizer();
    //lightup();
  //}*/
  FastLED.show();
  EVERY_N_MILLIS(500){
    random16_add_entropy(analogRead(0));
    Serial.print("FPS: ");
    Serial.println(LEDS.getFPS());
    //Serial.print(" Power: ");
    //Serial.println(calculate_unscaled_power_mW(leds, LEDtotal)/5/LEDbright);
  }
}

void barz(){
  //EVERY_N_MILLIS(8){
    int barwideness = DISPLAY_WIDTH/14;
    Heltec.display->clear();
    
    for(int i = 0; i < EQbins; i++){
      int barhightness = map(EQscaled[i], 0, LEDper, 0, DISPLAY_HEIGHT); 
      Heltec.display->fillRect(i*barwideness, DISPLAY_HEIGHT-barhightness, barwideness-1, barhightness); // void fillRect(uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
      
    }
    Heltec.display->display();
  //}
}

void justtext(){
  EVERY_N_MILLIS(8){
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->println(EQbuff[2]);
    //Heltec.display->println(EQbuff[2]);
    Heltec.display->display();
  }
}
void textbuffer(){
  EVERY_N_MILLIS(100){
    Heltec.display->clear();
    // Print to the screen
    
    Heltec.display->println(EQbuff[2]);
    // Draw it to the internal screen buffer
    Heltec.display->drawLogBuffer(0, 0);
    // Display it on the screen
    Heltec.display->display();
  }
}

void quadplexor(){
  for(int i = 0; i < LEDtotal; i++){
    leds[i] = CRGB(0,0,0);
  }
  for(int band = 0; band < EQbins; band++){
    byte z = map(EQscaled[band], 0, LEDper, 0, LEDper/2);
    for(int leng = 0; leng < z; leng++){                // Display as 00 11 22 33 44 55 66 66 55 44 33 22 11 00  CHSV(hue+leng*5-s*7, 255, 255); EQscaled[band]
      leds[XY(LEDstrips/2 -1  -band,   LEDper/2    -2   -leng)] = CHSV(hue + band*5 + leng*5, 255, 255);  // Top left
      leds[XY(LEDstrips/2     +band,   LEDper/2    -2   -leng)] = CHSV(hue + band*5 + leng*5, 255, 255);    // Top right
      leds[XY(LEDstrips/2 -1  -band,   LEDper/2    -1   +leng)] = CHSV(hue + band*5 + leng*5, 255, 255);  // Bottom left
      leds[XY(LEDstrips/2     +band,   LEDper/2    -1   +leng)] = CHSV(hue + band*5 + leng*5, 255, 255);    // Bottom right
    }
    if(z > 0){ //peaks
      leds[XY(LEDstrips/2 -1  -band,   LEDper/2    -1   -z )] = CHSV(255, 0, 255);  // Top left
      leds[XY(LEDstrips/2     +band,   LEDper/2    -1   -z )] = CHSV(255, 0, 255);    // Top right
      leds[XY(LEDstrips/2 -1  -band,   LEDper/2    -2   +z )] = CHSV(255, 0, 255);  // Bottom left
      leds[XY(LEDstrips/2     +band,   LEDper/2    -2   +z )] = CHSV(255, 0, 255);    // Bottom right
    }
  }
  hue++;
  FastLED.show();
}

void visualizer(){
  //FastLED.clear(); 
  for(int i = 0; i < LEDtotal; i++){
    leds[i] = CRGB(0,0,0);
  }
  for(int band = 0; band < EQbins; band++){
    for(int leng = 0; leng < EQscaled[band]; leng++){                // Display as 00 11 22 33 44 55 66 66 55 44 33 22 11 00  CHSV(hue+leng*5-s*7, 255, 255); EQscaled[band]
      leds[XY(LEDstrips/2+band, leng)] = CHSV(hue+leng*5, 255, 255);
      leds[XY(LEDstrips/2-1-band, leng)] = CHSV(hue+leng*5, 255, 255);
    }
  }
  hue++;
  FastLED.show();
}

void lightup(){
  FastLED.clear();
  for (int x = 0; x < LEDstrips; x++){
    for(int y = 0; y < LEDper; y++){
      leds[XY(x,y)] = CHSV(hue+y*80, 255, 255);
    }
  }
  hue++;
  FastLED.show();
}
