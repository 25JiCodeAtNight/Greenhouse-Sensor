#include <EEPROM.h>
#include <BLEDevice.h>
#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT11
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21

#define SERVICE_UUID "67e1caa4-8810-4324-8228-752b7a2639f9"        // random Service UUID
#define CHARACTERISTIC_UUID "6c4c233c-d35c-4487-8414-8bffe476e6be" // random Characteristic UUID

string deviceName = "Greenhouse-Sensor";
string sensorID = "";

DHT dht(DHTPIN, DHTTYPE);

class MyServerCallbacks : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        Serial.println("BLE Connected");
    };

    void onDisconnect(BLEServer *pServer)
    {
        Serial.println("BLE Disconnected");
    };
};

class MyCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();
        String value_s = value.c_str();
        int length = value_s.length();
        if (length != 16)
        {
            Serial.println("Wrong sensorID length!");
            return;
        }

        Serial.print("Write to EEPROM, SensorID: ");
        Serial.println(value_s);

        // Write to EEPROM
        EEPROM.begin(17);
        for (int i = 0; i < 16; i++)
        {
            EEPROM.write(i, value_s[i]);
        }
        EEPROM.commit();
        EEPROM.end();
    };
};

void setup()
{
    // Init Serial
    Serial.begin(9600);

    // Init Bluetooth
    BLEDevice::init(deviceName);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    pServer->setCallbacks(new MyServerCallbacks());
    pCharacteristic->setCallbacks(new MyCallbacks());
    pService->start();
    BLEDevice::startAdvertising();

    // Init DHT
    dht.begin();

    // Read from EEPROM
    EEPROM.begin(17);
    for (int i = 0; i < 16; i++)
    {
        sensorID += (char)EEPROM.read(i);
    }
    EEPROM.end();
    Serial.print("Read from EEPROM, SensorID: ");
    Serial.println(sensorID);
}

void loop()
{
    delay(4000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t))
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.println(" *C");
}
