#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <HTTPClient.h>
#include <PubSubClient.h>

const char *ssid = "WIFI SSID";
const char *password = "WIFI PASSWORD";

const char *mqtt_broker = "IPADRESS";   // broker address
const char *topic = "esp32/test";       // define topic
const char *mqtt_username = "USERNAME"; // username for authentication
const char *mqtt_password = "PASSWORD"; // password for authentication
const int mqtt_port = 1883;             // port of MQTT over TCP
WiFiClient espClient;
PubSubClient client(espClient);

#define RST_PIN 15 // Reset Pin for RC522, you can use any available pin
#define SS_PIN 33  // Slave Select Pin for RC522, connect to IO33 on Wemos S2 Mini

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create an instance of the MFRC522 RFID reader
MFRC522::MIFARE_Key key;

void setup()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    return;
  }

  // Define custom SPI settings
  SPI.begin(7, 9, 11, SS_PIN); // SCK=GPIO7, MISO=GPIO9, MOSI=GPIO11, SS=IO33

  mfrc522.PCD_Init();
  client.setServer(mqtt_broker, mqtt_port);
  while (!client.connected())
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
    {
    }
    else
    {
      delay(2000);
    }
  }
  // publish and subscribe
  client.publish(topic, "Hi EMQX I'm ESP32 ^^");
  client.subscribe(topic);
}

void loop()
{
  client.loop();

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
    return;

  String kartID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    // Serial.print(mfrc522.uid.uidByte[i], HEX);
    kartID += String(mfrc522.uid.uidByte[i], HEX);
  }
  if (client.connected())
  {
    client.publish(topic, kartID.c_str());
  }
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();
  delay(300);