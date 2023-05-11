#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


//Using this library for HX711 load cell amplifier: https://github.com/bogde/HX711
//Read pinout on data sheet here: https://www.es.co.th/Schemetic/PDF/ESP32.PDF

HX711 neckSensor;
const int NECK_DT = 32;
const int NECK_SCK = 33;
const float NECK_CF = 44.76350726;

HX711 armSensor;
const int ARM_DT = 25;
const int ARM_SCK = 26;
const float ARM_CF = 1.5;

HX711 chestSensor;
const int CHEST_DT = 34;
const int CHEST_SCK = 35;
const float CHEST_CF = 44.76350726;

const uint8_t TARE_READINGS = 100;
const uint8_t SCALE_READINGS = 20;


//Generate UUIDs using:
//https://www.uuidgenerator.net/
//ESP32 Bluetooth Hardware Address "C8:F0:9E:9C:87:52"
#define AMPLIFIER_SERVICE_UUID "6b8a5ab0-a8da-11ed-afa1-0242ac120002"
#define DATA_CHARACTERISTIC_UUID "70c21748-a8da-11ed-afa1-0242ac120002"

BLEServer *pServer = NULL; 
BLECharacteristic *dataCharacteristic = NULL;

//Using this library for Bluetooth Low Energy (BLE) functionality: https://github.com/nkolban/ESP32_BLE_Arduino/blob/master/examples/BLE_notify/BLE_notify.ino
bool deviceConnected = false;
bool oldDeviceConnected = false;
class MyServerCallbacks: public BLEServerCallbacks {
      void onConnect(BLEServer* pServer){
          deviceConnected = true;
      };
      void onDisconnect(BLEServer* pServer){
          deviceConnected = false;
      }
};


void setup() {
  Serial.begin(115200);

  rtc_clk_cpu_freq_set(RTC_CPU_FREQ_80M); //Need to slow down ESP32 from 240 MHz to 80 MHz to match HX711 clock speed.
  Serial.println("Initializing the neck sensor...");
  neckSensor.begin(NECK_DT, NECK_SCK);
  Serial.println("Calibrating neck sensor...");
  neckSensor.set_scale(NECK_CF); //Calibration factor derived from reading a known weight and calculating it as scale.read()/known weight in grams
  Serial.println("Taring neck sensor...");
  neckSensor.tare(TARE_READINGS); //Offsets scale by average of 10 readings with nothing on the scale.
  Serial.println("Setting up neck sensor...");
  while(!neckSensor.is_ready());
  Serial.println("Neck sensor is setup!");

  Serial.println("\nInitializing the arm sensor...");
  armSensor.begin(ARM_DT, ARM_SCK);
  Serial.println("Calibrating arm sensor...");
  armSensor.set_scale(ARM_CF); //Calibration factor derived from reading a known weight and calculating it as scale.read()/known weight in grams
  Serial.println("Taring arm sensor...");
  armSensor.tare(TARE_READINGS); //Offsets scale by average of 10 readings with nothing on the scale.
  Serial.println("Setting up arm sensor...");
  while(!armSensor.is_ready());
  Serial.println("Arm sensor is setup!");

  Serial.println("\nInitializing the chest sensor...");
  chestSensor.begin(CHEST_DT, CHEST_SCK);
  Serial.println("Calibrating chest sensor...");
  chestSensor.set_scale(CHEST_CF); //Calibration factor derived from reading a known weight and calculating it as scale.read()/known weight in grams
  Serial.println("Taring chest sensor...");
  //chestSensor.tare(TARE_READINGS); //Offsets scale by average of 10 readings with nothing on the scale.
  Serial.println("Setting up chest sensor...");
  //while(!chestSensor.is_ready());
  Serial.println("Chest sensor is setup!");

  Serial.println("\nStarting BLE work!");
  BLEDevice::init("ESP32");
  Serial.println("Creating BLE server...");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("Creating BLE Service...");
  BLEService *pService = pServer->createService(AMPLIFIER_SERVICE_UUID);
  
  Serial.println("Creating BLE Characteristic for 3 Data Readings");
  dataCharacteristic = pService->createCharacteristic(
                      DATA_CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor to allo(w for client (Android tablet) to enable or disable notifications.
  dataCharacteristic->addDescriptor(new BLE2902());

  //Start the service
  pService->start();
  Serial.println("Advertising...");
  //Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(AMPLIFIER_SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); //set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting for a a client connection to notify...");
}
void loop() {
  neckSensor.power_down();
  armSensor.power_down();
  chestSensor.power_down();
  //For calibration/printing only:
  /*
  float grams = neckSensor.get_units(SCALE_READINGS); //get_units() outputs a simple average for n readings.
  Serial.print("Choke Reading: "); Serial.print(grams, 2); Serial.println(" g");
  char string1_reading[15];
  dtostrf(grams, 10, 2, string1_reading);
  
  grams = armSensor.get_units(SCALE_READINGS); //get_units() outputs a simple average for n readings.
  Serial.print("Arm Bar Reading: "); Serial.print(grams, 2); Serial.println(" g");
  char string2_reading[15];
  dtostrf(grams, 10, 2, string2_reading);
  
  grams = chestSensor.get_units(SCALE_READINGS); //get_units() outputs a simple average for n readings.
  Serial.print("Strike Reading: "); Serial.print(grams, 2); Serial.println(" g");
  char string3_reading[15];
  dtostrf(grams, 10, 2, string3_reading);
  */
  //Notify changed value
  if(deviceConnected){
    neckSensor.power_up();
    //get_units() outputs a simple  average for n readings.
    int neckGrams = neckSensor.get_units(SCALE_READINGS);
    Serial.print("Neck Reading: "); Serial.print(neckGrams); Serial.println(" g");
    armSensor.power_up();
    int armGrams = armSensor.get_units(SCALE_READINGS);
    Serial.print("Arm Reading: "); Serial.print(armGrams); Serial.println(" g");
    chestSensor.power_up();
    int chestGrams = 0;
    Serial.print("Chest Reading: "); Serial.print(chestGrams); Serial.println(" g");
    char data_buffer[20]; //Maximum transmission for BLE is 20 characters
    sprintf(data_buffer, "%d\n%d\n%d\n", neckGrams, armGrams, chestGrams); //Send all three integers into string and use newline as delimiter
    dataCharacteristic->setValue(data_buffer);
    dataCharacteristic->notify();
    delay(3); //Bluetooth stack will go into congestion if too many packets are sent.
  }
  //Disconnecting
  if(!deviceConnected && oldDeviceConnected){
    Serial.println("Disconnecting...");
    delay(500); //Give the Bluetooth stack time to get things ready.
    pServer->startAdvertising();
    Serial.println("Start advertising again...");
    oldDeviceConnected = deviceConnected;
  }
  //Connecting
  if (deviceConnected && !oldDeviceConnected) {
      // do stuff here on connecting
      Serial.println("Connecting...");
      oldDeviceConnected = deviceConnected;
  }
}