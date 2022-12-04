#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

HX711 scale; //Using this library for HX711 load cell amplifier: https://github.com/bogde/HX711
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
const uint8_t tare_readings = 100;
const uint8_t scale_readings = 30;

BluetoothSerial SerialBLE;

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

  SerialBLE.begin("HX711_Scale");
  
  Serial.println("You can now pair HX711_Scale via bluetooth.");
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
  SerialBLE.print(grams, 2); SerialBLE.println(" g");
}

