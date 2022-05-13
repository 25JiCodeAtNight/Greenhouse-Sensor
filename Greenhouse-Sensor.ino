#include <EEPROM.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BLEDevice.h>
#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT11
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21

#define SERVICE_UUID "67e1caa4-8810-4324-8228-752b7a2639f9"  // random Service UUID
#define SENSORID_UUID "6c4c233c-d35c-4487-8414-8bffe476e6be" // random Characteristic UUID

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define DEVICE_NAME "Greenhouse-Sensor"
String sensorID = "";

#define SERVER_URL "http://test.com"

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
    BLEDevice::init(DEVICE_NAME);
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *SensorIDCharacteristic = pService->createCharacteristic(
        SENSORID_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pServer->setCallbacks(new MyServerCallbacks());
    SensorIDCharacteristic->setCallbacks(new WriteSesorIDCallbacks());
    pService->start();
    BLEDevice::startAdvertising();

    // Init DHT
    dht.begin();

    // Read SensorID from EEPROM
    EEPROM.begin(17);
    int i = 0;
    for (; i < 16; i++)
    {
        sensorID += (char)EEPROM.read(i);
    }
    Serial.print("Read from EEPROM, SensorID: ");
    Serial.println(sensorID);
    char c;
    int timeout_count = 0;
    EEPROM.end();

    // Connect Wi-Fi
    Serial.println("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}

void loop()
{
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

    // Send Data To Server
    String post_url = String(SERVER_URL) + "/v1/orders/sensor/submit?" + "sensorid=" + sensorID + "&" +"humidity=" + String(h) + "&" + "temperature=" + String(t);
    HTTPClient http;
    http.begin(post_url);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
            Serial.println("[HTTP] Data send successfully");
        else
            Serial.println("[HTTP] Data send failed");
    }
    delay(30000); // per 30s
}
