
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <DHT.h>           
#define DHTPIN 4            
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


#define temperatureCelsius
#define bleServerName  "202135536"      

#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#ifdef temperatureCelsius
  BLECharacteristic tempChar("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor     tempDesc(BLEUUID((uint16_t)0x2902));
#else
  BLECharacteristic tempChar("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor     tempDesc(BLEUUID((uint16_t)0x2902));
#endif
BLECharacteristic humChar("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);

bool deviceConnected = false;
unsigned long lastTime = 0;
const unsigned long timerDelay = 2000


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer*)   { deviceConnected = true; }
  void onDisconnect(BLEServer*){ deviceConnected = false; }
};


void setup() {
  Serial.begin(115200);
  dht.begin();

  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pService->addCharacteristic(&tempChar);
  tempDesc.setValue("DHT11 temperature");
  tempChar.addDescriptor(&tempDesc);

  pService->addCharacteristic(&humChar);
  humChar.addDescriptor(new BLE2902());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("Waiting for a client connection…");
}


void loop() {
  if (deviceConnected && millis() - lastTime > timerDelay) {
    float t = dht.readTemperature();     
    float h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT read failed!");
      lastTime = millis();
      return;
    }

#ifdef temperatureCelsius
    char tBuf[6];  dtostrf(t, 6, 2, tBuf);
    tempChar.setValue(tBuf);   tempChar.notify();
    Serial.print("T: "); Serial.print(t); Serial.print(" °C  ");
#else
    float tF = t * 1.8 + 32;
    char tBuf[6];  dtostrf(tF, 6, 2, tBuf);
    tempChar.setValue(tBuf);   tempChar.notify();
    Serial.print("T: "); Serial.print(tF); Serial.print(" °F  ");
#endif

    char hBuf[6];  dtostrf(h, 6, 2, hBuf);
    humChar.setValue(hBuf);    humChar.notify();
    Serial.print("H: "); Serial.print(h); Serial.println(" %");

    lastTime = millis();
  }
}
