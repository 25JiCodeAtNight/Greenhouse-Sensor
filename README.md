# Greenhouse-Sensor

A home-made temperature and humidity sensor which can monitor the greenhouse and connect to GreenhouseMonitor WeChat miniprogram.

## List of Materials

must:

- ESP32C3 demo board
- DHT 11/21/22 sensor
- Jumper wire (at least 3, about 5cm long is good)
- USB Type C cable
- Charger (5V 1A is enough)
- Wi-Fi connection (for connecting to server)

option:

- 3D printer (for housing)

## Flash  Firmware

### Modify Firmware

Open file `Greenhouse-Sensor.ino`.

Change the following lines with your settings:

```
#define DHTTYPE DHT11       // Choose the sensor you use
//#define DHTTYPE DHT22
//#define DHTTYPE DHT21
...
#define WIFI_SSID ""        // Your Wi-Fi SSID
#define WIFI_PASSWORD ""    // Your Wi-Fi password
...
#define SERVER_URL "http://test.com"    // Server URL
```

Then use Arduino to compile and flash the firmware.

## Sensor Housing

The housing is designed for HeZhou(合宙) ESP32C3 demo board.

There're CAD files and you can try to modify them.

See [Sensor Housing Instuction](./sensor%20housing/README.md)