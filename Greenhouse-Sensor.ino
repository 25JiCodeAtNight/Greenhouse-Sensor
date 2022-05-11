#include <EEPROM.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT11
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21

#define SERVICE_UUID "67e1caa4-8810-4324-8228-752b7a2639f9"  // random Service UUID
#define SensorID_UUID "6c4c233c-d35c-4487-8414-8bffe476e6be" // random Characteristic UUID
#define WIFI_UUID "6c4c233c-d35c-4487-8414-8bffe476e6bf"     // random Characteristic UUID

char *wifi_ssid = "";
char *wifi_password = "";
#define MAX_WIFI_LENGTH 30

string deviceName = "Greenhouse-Sensor";
string sensorID = "";

#define SERVER_URL = "http://test.com/api/v1/sensor"

DHT dht(DHTPIN, DHTTYPE);

class BLEServerCallbacks : public BLEServerCallbacks
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

class WriteSesorIDCallbacks : public BLECharacteristicCallbacks
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
        EEPROM.begin(160);
        for (int i = 0; i < 16; i++)
        {
            EEPROM.write(i, value_s[i]);
        }
        EEPROM.commit();
        EEPROM.end();
    };
};

class WriteWiFiCallbacks : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string value = pCharacteristic->getValue();    // Frontend return format: "SSID;PASSWORD;"
        String value_s = value.c_str();
        int length = value_s.length();
        // Write to EEPROM
        EEPROM.begin(160);
        for (int i = 16; i < length; i++)
        {
            EEPROM.write(i, value_s[i]);
        }
        EEPROM.commit();
        EEPROM.end();
    }
}

void
setup()
{
    // Init Serial
    Serial.begin(9600);

    // Init Bluetooth
    BLEDevice::init(deviceName);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *SensorIDCharacteristic = pService->createCharacteristic(
        SensorID_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);
    BLECharacteristic *WIFICharacteristic = pService->createCharacteristic(
        WIFI_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pServer->setCallbacks(new BLEServerCallbacks());
    SensorIDCharacteristic->setCallbacks(new WriteSesorIDCallbacks());
    WIFICharacteristic->setCallbacks(new WriteWiFiCallbacks());
    pService->start();
    BLEDevice::startAdvertising();

    // Init DHT
    dht.begin();

    // Read from EEPROM
    EEPROM.begin(160);
    int i = 0;
    for (; i < 16; i++)
    {
        sensorID += (char)EEPROM.read(i);
    }
    Serial.print("Read from EEPROM, SensorID: ");
    Serial.println(sensorID);
    char c = '';
    int timeout_count = 0;
    // Read Wi-Fi SSID
    while (c != ';')
    {
        c = EEPROM.read(i);
        wifi_ssid += c;
        i++;
        timeout_count++;
        if (timeout_count > MAX_WIFI_LENGTH)
        {
            Serial.println("Could not read SSID from EEPROM!");
            break;
        }
    }
    if (timeout_count < MAX_WIFI_LENGTH)
    {
        Serial.print("Read from EEPROM, SSID: ");
        Serial.println(wifi_ssid);
    }
    // Read Wi-Fi Password
    timeout_count = 0;
    i++;
    while (c != ';')
    {
        c = EEPROM.read(i);
        wifi_ssid += c;
        i++;
        timeout_count++;
        if (timeout_count > MAX_WIFI_LENGTH)
        {
            Serial.println("Could not read password from EEPROM!");
            break;
        }
    }
    if (timeout_count < MAX_WIFI_LENGTH)
    {
        Serial.print("Read from EEPROM, Password: ");
        Serial.println(wifi_password);
    }
    EEPROM.end();
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
