#ifndef EQ_h
#define EQ_h

//////////////////////////////////////////////////////////
  /*
  EQsampletimer = round(1000000 * (1.0 / EQsamplefreq));
  pinMode(ledPin, OUTPUT);
  */

// FFT
#include <arduinoFFT.h>
#include <Statistic.h>
#define EQsamples      512            // Must be a power of 2
#define EQsamplefreq   40000 //40000  // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define EQsourcepin    35             // Signal in on this pin
#define EQbins         14             // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands

unsigned int EQsampletimer;
byte EQpeak[EQbins] = {0};              // The length of these arrays must be >= NUM_BANDS
int EQoldBarHeights[EQbins] = {0};
uint16_t EQnoisecutoff[EQbins];
double EQreal[EQsamples];
double EQimag[EQsamples];
unsigned long EQtimer;
arduinoFFT FFT = arduinoFFT(EQreal, EQimag, EQsamples, EQsamplefreq);

#define EQbins 14
uint32_t EQbuff[EQbins];
uint32_t EQdecay[EQbins];
uint32_t EQmaxes[EQbins];
uint32_t EQmins[EQbins] = {4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295, 4294967295};
Statistic EQstatstotal[EQbins];
int EQscaled[EQbins];
float EQflatdecline[EQbins];

uint32_t EQaverage[EQbins];
uint32_t EQstDev[EQbins];

bool EQbeat;
uint8_t EQbeatDetected[EQbins];
int EQbeatInterval[EQbins];
int EQbeatIntervalOld[EQbins];
int EQconstantBeatCounter[EQbins];
//elapsedMillis EQbeatTimer[EQbins];
const int ledPin =  25;
int ledState = LOW; 

//0: (0-4977.45), 1: (0-2610.57), 2: (0-2886.10), 3: (0-2529.93), 4: (0-1343.32), 5: (0-838.57), 6: (0-1746.94), 7: (0-1318.85), 8: (1-1404.55), 9: (0-1724.66), 10: (1-2038.04), 11: (2-2928.59), 12: (4-4228.29), 13: (0-5438.49), 

                /////////////0////////1////////2////////3////////4////////5////////6////////7///////8//////9////////10//////11///////12///////13//
uint32_t noisethresh[14] = { 5250,   3410,    3400,    2200,     1200,    1200,    1746,    1318,   1404,   1724,    2038,   2928,   4228,     6060 };
//////////////////////////////0////////1////////2////////3////////4////////5////////6///////7////////8//////9///////10///////11///////12///////13//
uint32_t mintops[14] =     {15000,  22896,    15000,   15000,    15000,  15000,  15000,   15000,  15000,  15000,   15000,  15000,   15000,    15000 };

void EQget(){
  // Reset buffer
  for (int i = 0; i < EQbins; i++){
    EQbuff[i] = 0;
  }
  
  // Sample the audio pin
  for (int i = 0; i < EQsamples; i++) {
    EQtimer = micros();
    EQreal[i] = analogRead(EQsourcepin); // A conversion takes about 9.7uS on an ESP32
    EQimag[i] = 0;
    while (micros() < (EQtimer + EQsampletimer)) { /* chill */ }
  }

  // Compute FFT
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  // Analyse FFT results
  for (int i = 2; i < (EQsamples/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
    /*
    //14 bands, 256 samples
    if (i <= 2)               EQbuff[0]  += (int)EQreal[i];
    if (i == 3)               EQbuff[1]  += (int)EQreal[i];
    if (i >= 4 && i <= 5)     EQbuff[2]  += (int)EQreal[i];
    if (i >= 6 && i <= 7)     EQbuff[3]  += (int)EQreal[i];
    if (i >= 8 && i <= 10)    EQbuff[4]  += (int)EQreal[i];
    if (i >= 11 && i <= 14)   EQbuff[5]  += (int)EQreal[i];
    if (i >= 15 && i <= 19)   EQbuff[6]  += (int)EQreal[i];
    if (i >= 20 && i <= 25)   EQbuff[7]  += (int)EQreal[i];
    if (i >= 26 && i <= 33)   EQbuff[8]  += (int)EQreal[i];
    if (i >= 34 && i <= 44)   EQbuff[9]  += (int)EQreal[i];
    if (i >= 45 && i <= 58)   EQbuff[10]  += (int)EQreal[i];
    if (i >= 59 && i <= 76)   EQbuff[11]  += (int)EQreal[i];
    if (i >= 77 && i <= 99)   EQbuff[12]  += (int)EQreal[i];
    if (i >= 100)             EQbuff[13]  += (int)EQreal[i];
    */
     
    //14 bands, 512 samples
    if (i <= 2)               EQbuff[0]  += (int)EQreal[i];
    if (i == 3)               EQbuff[1]  += (int)EQreal[i];
    if (i >= 4 && i <= 5)     EQbuff[2]  += (int)EQreal[i];
    if (i >= 6 && i <= 8)     EQbuff[3]  += (int)EQreal[i];
    if (i >= 9 && i <= 12)    EQbuff[4]  += (int)EQreal[i];
    if (i >= 13 && i <= 17)   EQbuff[5]  += (int)EQreal[i];
    if (i >= 18 && i <= 24)   EQbuff[6]  += (int)EQreal[i];
    if (i >= 25 && i <= 34)   EQbuff[7]  += (int)EQreal[i];
    if (i >= 35 && i <= 48)   EQbuff[8]  += (int)EQreal[i];
    if (i >= 49 && i <= 67)   EQbuff[9]  += (int)EQreal[i];
    if (i >= 68 && i <= 94)   EQbuff[10]  += (int)EQreal[i];
    if (i >= 95 && i <= 131)  EQbuff[11]  += (int)EQreal[i];
    if (i >= 132 && i <= 183) EQbuff[12]  += (int)EQreal[i];
    if (i >= 184)             EQbuff[13]  += (int)EQreal[i];
    
    /*
    //14 bands, 12kHz top band
    if (i <= 2)               EQbuff[0]  += (int)EQreal[i];
    if (i == 3)               EQbuff[1]  += (int)EQreal[i];
    if (i == 4)               EQbuff[2]  += (int)EQreal[i];
    if (i >= 5 && i <= 6)     EQbuff[3]  += (int)EQreal[i];
    if (i >= 7 && i <= 11)    EQbuff[4]  += (int)EQreal[i];
    if (i >= 12 && i <= 18)   EQbuff[5]  += (int)EQreal[i];
    if (i >= 19 && i <= 29)   EQbuff[6]  += (int)EQreal[i];
    if (i >= 30 && i <= 45)   EQbuff[7]  += (int)EQreal[i];
    if (i >= 46 && i <= 68)   EQbuff[8]  += (int)EQreal[i];
    if (i >= 69 && i <= 103)  EQbuff[9]  += (int)EQreal[i];
    if (i >= 104 && i <= 154) EQbuff[10]  += (int)EQreal[i];
    if (i >= 155 && i <= 230) EQbuff[11]  += (int)EQreal[i];
    if (i >= 231 && i <= 343) EQbuff[12]  += (int)EQreal[i];
    if (i >= 344)             EQbuff[13]  += (int)EQreal[i];
    */
    
    /*
    //14 bands, 12kHz top band
    if (i == 0 )           bandValues[0]  += (int)vReal[i];
    if (i >= 1 && i <= 1) bandValues[1]  += (int)vReal[i];
    if (i >= 2 && i <= 3) bandValues[2]  += (int)vReal[i];
    if (i >= 4 && i <= 6) bandValues[3]  += (int)vReal[i];
    if (i >= 7 && i <= 11) bandValues[4]  += (int)vReal[i];
    if (i >= 12 && i <= 18) bandValues[5]  += (int)vReal[i];
    if (i >= 19 && i <= 29) bandValues[6]  += (int)vReal[i];
    if (i >= 30 && i <= 45) bandValues[7]  += (int)vReal[i];
    if (i >= 46 && i <= 68) bandValues[8]  += (int)vReal[i];
    if (i >= 69 && i <= 103) bandValues[9]  += (int)vReal[i];
    if (i >= 104 && i <= 154) bandValues[10]  += (int)vReal[i];
    if (i >= 155 && i <= 230) bandValues[11]  += (int)vReal[i];
    if (i >= 231 && i <= 343) bandValues[12]  += (int)vReal[i];
    if (i >= 344 && i <= 511) bandValues[13]  += (int)vReal[i];
    */    
  }
}
  
void EQprintone(int target){
  
  Serial.print("EQbuff: ");
  Serial.println(EQbuff[target]);
  
  Serial.print("EQmins: ");
  Serial.println(EQmins[target]);
  
  Serial.print("EQmaxes: ");
  Serial.println(EQmaxes[target]);
  
  Serial.print("EQscaled: ");
  Serial.println(EQscaled[target]);

  Serial.print("EQdecay: ");
  Serial.println(EQdecay[target]);

  Serial.print("EQflatdecline: ");
  Serial.println(EQflatdecline[target]);
  

  Serial.println();
}

void EQprintall(){

  for(int i = 0; i < EQbins; i++){
    Serial.print(EQbuff[i]);
    Serial.print(",");
  }
    /*
  for(int i = 0; i < EQbins; i++){
    Serial.print(EQscaled[i]);
    Serial.print(",");
  }*/
  Serial.println();
}

void EQprintallmaxes(){
  //for(int i = 0; i < EQbins; i++){
   // Serial.print(EQscaled[i]);
  //  Serial.print(",");
  //}
    /*
  for(int i = 0; i < EQbins; i++){
    Serial.print(EQscaled[i]);
    Serial.print(",");
  }*/
  //Serial.println();
  for(int i = 0; i < EQbins; i++){
    Serial.print(i);
    Serial.print(": (");
    Serial.print(EQscaled[i]);
    Serial.print("-");
    Serial.print(EQmaxes[i]);
    Serial.print("), ");
  }
    /*
  for(int i = 0; i < EQbins; i++){
    Serial.print(EQscaled[i]);
    Serial.print(",");
  }*/
  //Serial.println();
}

void EQbeatBlink(){
  if (EQbeat == true){
    ledState = HIGH;
  } else {
    ledState = LOW;
  }
  digitalWrite(ledPin, ledState);
}

void EQnoisegate(){
  
  for(int i = 0; i < EQbins; i++){
    //uint32_t x = _max(noisethresh[i], EQmins[i]);
    //uint32_t y = _max(mintops[i], );
    uint32_t a = _max(noisethresh[i], EQmins[i]);             // Use the highest min value coded or caluclated
    uint32_t x = _max(a, 0); // Use the highest min value coded, calculated, or 2.5 std devs away from average  uint32_t x = max(a, (EQaverage[i] - (2.5 *EQstDev[i]))); // Use the highest min value coded, calculated, or 2.5 std devs away from average
    uint32_t b = _max(mintops[i], EQmaxes[i]);                // Use the highest max value coded or calculated
    uint32_t y = _max(b, (EQaverage[i] + (2.5 *EQstDev[i]))); // Use the highest max value coded, calculated, or 2.5 std devs away from average
    //uint32_t z = _max(EQbuff[i], EQdecay[i]);
    uint32_t z = _max(EQbuff[i], 0);                          
 
    if(z >= noisethresh[i]){
      EQscaled[i] = map(z, x, y, 0, LEDper);          //(input, inmin, inmax, outmin, outmax)
    } else {
      EQscaled[i] = 0;
    } 
    if(EQscaled[i] >= EQflatdecline[i]){
      EQflatdecline[i] = EQscaled[i];
    } else {
      EQscaled[i] = EQflatdecline[i];
      EQflatdecline[i]-=5;
    }
  } 
}

void EQstats(){ 
  for(int i = 0; i < EQbins; i++){    
    if( EQbuff[i] >= EQdecay[i]){              // Reset decaying value to smooth out drops
      EQdecay[i] = EQbuff[i];
    } else {
      EQdecay[i] = EQdecay[i] - _max(1, EQdecay[i] * .2);              // Decay must be relative to the frequency, could change to flat value
    }
  
    if( EQbuff[i] >= EQmaxes[i] ){             // Set maximum volume level for scaling      
      EQmaxes[i] = EQbuff[i];
    } else {           
      EQmaxes[i] = EQmaxes[i] - _max(1, EQmaxes[i] * .001);     // need to make this based on std deviation     
    }
  
    if( EQbuff[i] <= EQmins[i] ){             // Set minimum volume level for scaling   
      EQmins[i] = EQbuff[i];
    } else {
      EQmins[i] = EQmins[i] + _max(.000001, EQaverage[i]*.005);    // Can't be zero                                                         // need to make this based on std deviation
    }  
          
    EQstatstotal[i].add(EQbuff[i]);  // Take stats for beat detection
    EQaverage[i] = EQstatstotal[i].average();
    EQstDev[i] = EQstatstotal[i].pop_stdev();
  }
}


    //uint32_t a = (255 - EQ7avg[s])/2;                          // How much handicapping should be applied based on how low avg is, increase divisor to decrease strength
    //int fq = 255*(log(1+(EQbuff[target]/255)*a)/log(1+a));         // Apply handicapping 
    //EQ7fqamp[s] = map(fq, 0, 255, 0, LEDper - 1);
    

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void EQprintDetectedBeats() {
  for(int i = 0; i < EQbins; i++){
    // These print statements will print the beat status of each bin
    if (i == EQbins - 1) {
      Serial.println(EQbeatDetected[i]);
    }
    else {
      Serial.print(EQbeatDetected[i]);
      Serial.print("\t");
    }
  }
}

void EQbeatDetection() {
  for(int i = 0; i < EQbins; i++){
    // 0 = no beat detected
    // 1 = beat hasn't dropped / reset yet
    // 2 = beat detected
    // Beat is detected here. Must be greater than the average+(2.3*st.dev) and greater than 0.004212 which may need to be adjusted
    if (EQbuff[i] > EQaverage[i] + 2.8 * EQstDev[i] && EQbuff[i] > noisethresh[i] + 2.8 * EQstDev[i] ) {
      if (EQbeatDetected[i] == 2){
        EQbeatDetected[i] = 1;
      }
      if (EQbeatDetected[i] == 0){
        EQbeatDetected[i] = 2;
        //Serial.print(i);
        //Serial.print(": ");
        //Serial.print(EQbuff[i]);
        //Serial.print(", ");
      }
      // if it's == 1, it stays 1.
    }
    // This is where 1's get reset to 0. If beat is not detected in that frequency, set it's status to 0.
    else {
      if (EQbeatDetected[i] == 2){
        EQbeatDetected[i] = 1;
      }
      if (EQbeatDetected[i] == 1) {
        // else, don't be ready for the next beat until the frequency has hit the running average.
        // This prevents multiple beats being triggered when they aren't well defined
        if (EQbuff[i] <= EQaverage[i]){
          EQbeatDetected[i] = 0;
        }
      }
    }
  }
}

void EQbeatBuckets(){
  byte sum = 0;
  bool hit = false;
  EQbeat = false;
  for(int i = 0; i < 4; i++){
    sum = sum + EQbeatDetected[i];
    if(EQbeatDetected[i] == 2){
      hit = true;
    }
  }
  if(sum >= 2 && hit == true){
    EQbeat = true;
    //Serial.println("beat");
  } else {
   //Serial.println("nope");
  }
}


void EQproc(){
  extern int loopval;
  //EVERY_N_SECONDS_I(shufloop, STATEshuffleinterval){
  EVERY_N_MILLIS_I(looper, loopval){     
    EQget();
    //EQstats();
    //EQnoisegate();
    //EQbeatDetection();    
    //EQbeatBuckets();
    //EQbeatBlink();
    //EQprintone();
    //EQprintall();
  }
  EVERY_N_SECONDS(2){
    //loopval++;
    looper.setPeriod(loopval);
  }
}


uint16_t XY(byte x, byte y){
  uint16_t LEDaddress = x*LEDper+y;
  return LEDaddress;
}

/*
void beatTiming(int i){
  //int beatTimerStandin = beatTimer[i]; 
  if (EQbeatDetected[i] == 2) { // if beat has been detected
    if (EQbeatTimer[i] < 200){ // and it isn't 200 ms from the last one
      EQbeatDetected[i] = 1; // but if it is, cancel the beat
    }
    else {
      EQbeatInterval[i] = EQbeatTimer[i];  // a beat has been detected. Get the time since last beat.
      // could use absolute value below
      if ((EQbeatIntervalOld[i] - EQbeatInterval[i]) < 30 && (EQbeatIntervalOld[i] - EQbeatInterval[i]) > -30){ // If the time between the new interval and old interval is less than 30 ms 
        EQconstantBeatCounter[i]++;
      }
      EQbeatTimer[i] = 0;
      EQbeatIntervalOld[i] = EQbeatInterval[i];
    }
  }
  
  if (EQconstantBeatCounter[i] > 0 && EQbeatTimer[i] > EQbeatInterval[i] + 50){
    EQconstantBeatCounter[i] = 0; // clears beat counter when more time than the beat has passed
  }

  // These print statements will print the constant beat counter of each bin
  if (i == EQbins - 1){
    Serial.println(EQconstantBeatCounter[i]);
  }
  else {
    Serial.print(EQconstantBeatCounter[i]);
    Serial.print("\t");
  }

  if ( i < 4) {
    if (EQconstantBeatCounter[i] > EQmaxConstBeat) {
      EQmaxConstBeat = EQconstantBeatCounter[i];
      EQconstBeatBin = i;
    }
  }
  if (EQmaxConstBeat > 10){
    EQconstBeat = true;
  }
  else {
    EQconstBeat = false;
  }
}
    */
    
/*
 * // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {
    
    // Scale the bars for the display
    int barHeight = bandValues[band] / AMPLITUDE;
    if (barHeight > TOP) barHeight = TOP;

    // Small amount of averaging between frames
    barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

    // Move peak up
    if (barHeight > peak[band]) {
      peak[band] = min(TOP, barHeight);
    }


    EVERY_N_SECONDS(10){
      randle = random8(6);
    }
    switch (randle) {
      FastLED.clear();
      case 0:
        rainbowBars(band, barHeight);
        break;
      case 1:
        // No bars on this one
        break;
      case 2:
        purpleBars(band, barHeight);
        break;
      case 3:
        centerBars(band, barHeight);
        break;
      case 4:
        changingBars(band, barHeight);
        break;
      case 5:
        waterfall(band);
        break;
    }

    // Draw peaks
    switch (randle) {
      case 0:
        whitePeak(band);
        break;
      case 1:
        outrunPeak(band);
        break;
      case 2:
        whitePeak(band);
        break;
      case 3:
        // No peaks
        break;
      case 4:
        // No peaks
        break;
      case 5:
        // No peaks
        break;
    }

    // Save oldBarHeights for averaging later
    oldBarHeights[band] = barHeight;
  }
  
  // Decay peak
  EVERY_N_MILLISECONDS(60) {
    for (byte band = 0; band < NUM_BANDS; band++) 
      if (peak[band] > 0) peak[band] -= 1;
    colorTimer++;
  }

  // Used in some of the patterns
  EVERY_N_MILLISECONDS(10) {
    colorTimer++;
  }
  
  FastLED.show();
}

// PATTERNS BELOW //

void rainbowBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY(x,y)] = CHSV((x / BAR_WIDTH) * (255 / NUM_BANDS), 255, 255); 
    }
  }
}

void purpleBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY(x,y)] = ColorFromPalette(purplePal, y * (255 / barHeight));
    }
  }
}

void changingBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    for (int y = 0; y < barHeight; y++) {
      leds[XY(x,y)] = CHSV(y * (255 / kMatrixHeight) + colorTimer, 255, 255); 
    }
  }
}

void centerBars(int band, int barHeight) {
  int xStart = BAR_WIDTH * band;
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    if (barHeight % 2 == 0) barHeight--;
    int yStart = ((kMatrixHeight - barHeight) / 2 );
    for (int y = yStart; y <= (yStart + barHeight); y++) {
      int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
      leds[XY(x,y)] = ColorFromPalette(heatPal, colorIndex);
    }
  }
}

void whitePeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    leds[XY(x,peakHeight)] = CRGB::White;
  }
}

void outrunPeak(int band) {
  int xStart = BAR_WIDTH * band;
  int peakHeight = peak[band];
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    //leds[XY(x,peakHeight)] = CHSV(peakHeight * (255 / kMatrixHeight), 255, 255);
    leds[XY(x,peakHeight)] = ColorFromPalette(outrunPal, peakHeight * (255 / kMatrixHeight));
  }
}

void waterfall(int band) {
  int xStart = BAR_WIDTH * band;
  double highestBandValue = 60000;        // Set this to calibrate your waterfall

  // Draw bottom line
  for (int x = xStart; x < xStart + BAR_WIDTH; x++) {
    leds[XY(x,0)] = CHSV(constrain(map(bandValues[band],0,highestBandValue,160,0),0,160), 255, 255);
  }

  // Move screen up starting at 2nd row from top
  if (band == NUM_BANDS - 1){
    for (int y = kMatrixHeight - 2; y >= 0; y--) {
      for (int x = 0; x < kMatrixWidth; x++) {
        leds[XY(x,y+1)] = leds[XY(x,y)];
      }
    }
  }
}
*/
#endif
