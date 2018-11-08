#include <Wire.h>
#include "arduinoFFT.h" 
arduinoFFT FFT = arduinoFFT();

#include "SSD1306.h" 
SSD1306 display(0x3c, 5,4); 
/////////////////////////////////////////////////////////////////////////
#define SAMPLES 512              
#define SAMPLING_FREQUENCY 40000 
#define amplitude 200            
unsigned int sampling_period_us;
unsigned long microseconds;
byte peak[] = {0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime, oldTime;
/////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Wire.begin(5,4); // SDA, SCL
  display.init();
  display.setFont(ArialMT_Plain_10);
  display.flipScreenVertically(); // Adjust to suit or remove
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}

void loop() {
  display.clear();
  display.drawString(0,0,"0.1 0.2 0.5 1K  2K  4K  8K");
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros()-oldTime;
    oldTime = newTime;
    vReal[i] = analogRead(A0); // A conversion takes about 1uS on an ESP32
    vImag[i] = 0;
    while (micros() < (newTime + sampling_period_us)) { /* do nothing to wait */ }
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
  for (int i = 2; i < (SAMPLES/2); i++){ // Don't use sample 0 and only first SAMPLES/2 are usable. Each array eleement represents a frequency and its value the amplitude.
    if (vReal[i] > 2000) { // Add a crude noise filter, 10 x amplitude or more
      if (i<=2 )             displayBand(0,(int)vReal[i]/amplitude); // 125Hz
      if (i >3   && i<=5 )   displayBand(1,(int)vReal[i]/amplitude); // 250Hz
      if (i >5   && i<=7 )   displayBand(2,(int)vReal[i]/amplitude); // 500Hz
      if (i >7   && i<=15 )  displayBand(3,(int)vReal[i]/amplitude); // 1000Hz
      if (i >15  && i<=30 )  displayBand(4,(int)vReal[i]/amplitude); // 2000Hz
      if (i >30  && i<=53 )  displayBand(5,(int)vReal[i]/amplitude); // 4000Hz
      if (i >53  && i<=200 ) displayBand(6,(int)vReal[i]/amplitude); // 8000Hz
      if (i >200           ) displayBand(7,(int)vReal[i]/amplitude); // 16000Hz
      //Serial.println(i);
    }
    for (byte band = 0; band <= 6; band++) display.drawHorizontalLine(18*band,64-peak[band],14);
  }
  if (millis()%4 == 0) {for (byte band = 0; band <= 6; band++) {if (peak[band] > 0) peak[band] -= 1;}} // Decay the peak
  display.display();
}



void displayBand(int band, int dsize){
  int dmax = 50;
  if (dsize > dmax) dsize = dmax;
  if (band == 7) display.drawHorizontalLine(18*6,0, 14);
  for (int s = 0; s <= dsize; s=s+2){display.drawHorizontalLine(18*band,64-s, 14);}
  if (dsize > peak[band]) {peak[band] = dsize;}
}


