#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <stdlib.h>

//Using this library for HX711 load cell amplifier: https://github.com/bogde/HX711
HX711 scale; 
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
const uint8_t tare_readings = 100;
const uint8_t scale_readings = 30;

//Using this library for Bluetooth Low Energy (BLE) functionality: https://github.com/nkolban/ESP32_BLE_Arduino/tree/master/src
BLEServer *pServer; 
BLEService *pService;
BLECharacteristic *pCharacteristic;
BLEAdvertising *pAdvertising;

//Generate UUIDs using:
//https://www.uuidgenerator.net/
//ESP32 Bluetooth Hardware Address "C8:F0:9E:9C:87:52"
#define SERVICE_UUID "6b8a5ab0-a8da-11ed-afa1-0242ac120002"
#define CHARACTERISTIC_UUID "70c21748-a8da-11ed-afa1-0242ac120002"

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

    Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("Jon");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService ->createCharacteristic(
                                                  CHARACTERISTIC_UUID,
                                                  BLECharacteristic::PROPERTY_READ |
                                                  BLECharacteristic::PROPERTY_WRITE
                                                );
  pCharacteristic->setValue("ESP32 connected!");
  pService->start();
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); //Minimum interval time
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it on Bluetooth scanner!");
}
void loop() {
  scale.power_down();
  Serial.println("\nWeigh object now.");
  delay(1000);
  scale.power_up();
  char buff[15];
  float grams = scale.get_units(scale_readings); //get_units() outputs a simple average for n readings.
  if(grams < 4.5){ //Avoid outputting when nothing is on the scale (output is around 1-4.5 g when nothing is on the scale).  
    grams = 0.0;
  }
  Serial.print("Reading: "); Serial.print(grams, 2); Serial.println(" g");
  dtostrf(grams, 10, 2, buff);
  pCharacteristic->setValue(buff);
}