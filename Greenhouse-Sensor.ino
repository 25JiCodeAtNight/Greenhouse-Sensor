#include "DHT.h"

#define DHTPIN 12
#define DHTTYPE DHT11
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21

DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(9600);

    dht.begin();
}

void loop() {
    delay(2000);
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
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
