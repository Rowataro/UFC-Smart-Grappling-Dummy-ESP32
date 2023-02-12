#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//Generate UUIDs using:
//https://www.uuidgenerator.net/
#define SERVICE_UUID "6b8a5ab0-a8da-11ed-afa1-0242ac120002"
#define CHARACTERISTIC_UUID "70c21748-a8da-11ed-afa1-0242ac120002"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  BLEDevice::init("Jon");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService ->createCharacteristic(
                                                  CHARACTERISTIC_UUID,
                                                  BLECharacteristic::PROPERTY_READ |
                                                  BLECharacteristic::PROPERTY_WRITE
                                                );
  pCharacteristic->setValue("123");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); //Minimum interval time?
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it on your phone!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
}
