#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"

HX711 scale; //Using this library for HX711 load cell amplifier: https://github.com/bogde/HX711
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
const uint8_t tare_readings = 100;
const uint8_t scale_readings = 30;

void setup() {
  Serial.begin(115200);
  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M); //Need to slow down ESP32 from 240 MHz to 80 MHz to match HX711 clock speed.

  Serial.println("Initializing the scale...");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println("Calibrating scale...");
  scale.set_scale(44.76350726); //Calibration factor derived from reading a known weight and calculating it as scale.read()/known weight in grams
  Serial.println("Taring scale...");
  scale.tare(tare_readings); //Offsets scale by average of 10 readings with nothing on the scale.
  Serial.println("Setting up scale...");
  while(!scale.is_ready());
  Serial.println("Scale is setup! ");

}

void loop() {
  scale.power_down();
  Serial.println("\nWeigh object now.");
  delay(1000);
  scale.power_up();
  float grams = scale.get_units(scale_readings); //get_units() outputs a simple average for n readings.
  if(grams < 4.5){ //Avoid outputting when nothing is on the scale (output is around 1-4.5 g when nothing is on the scale).  
    grams = 0.0;
  }
  Serial.print("Reading: "); Serial.print(grams, 2); Serial.println(" g");
}

