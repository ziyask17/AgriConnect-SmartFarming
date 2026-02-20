/****************************************************
 * AgriConnect - Smart Farming with IoT (FINAL VERSION)
 * Features:
- Connect to WiFi
- Connect to MQTT broker (Raspberry Pi / Mosquitto)
- Publish sensor data (soil, temp, humidity)
- Auto pump control based on soil moisture threshold
- Manual pump override via MQTT topic: agriConnect/pumpControl
- Send data to ThingSpeak via HTTP

 * Soil Sensor (DIGITAL):
 *   HIGH = DRY
 *   LOW  = WET
 *
 * Relay Module (ACTIVE-LOW):
 *   LOW  = Pump ON
 *   HIGH = Pump OFF
 ****************************************************/

#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <DHT.h>

/********** WIFI **********/
const char* WIFI_SSID     = "OPPO Reno13 5G 1DAE";
const char* WIFI_PASSWORD = "12345678";

/********** MQTT **********/
const char* MQTT_BROKER  = "10.251.160.53";   // Raspberry Pi IP
const uint16_t MQTT_PORT = 1883;

/********** THINGSPEAK **********/
String thingspeakApiKey = "FGPWS2PVTPTF2JTL";
const char* THINGSPEAK_SERVER = "http://api.thingspeak.com";

/********** INTERVAL **********/
const unsigned long READ_INTERVAL_MS = 15000;

/********** MQTT TOPICS **********/
const char* TOPIC_SOIL1 = "agriConnect/farm1";
const char* TOPIC_SOIL2 = "agriConnect/farm2";

const char* TOPIC_PUMP1_STATUS = "agriConnect/pump1/status";
const char* TOPIC_PUMP2_STATUS = "agriConnect/pump2/status";

const char* TOPIC_PUMP1_CTRL   = "agriConnect/pump1/control";
const char* TOPIC_PUMP2_CTRL   = "agriConnect/pump2/control";

const char* TOPIC_TEMP = "agriConnect/temperature";
const char* TOPIC_HUM  = "agriConnect/humidity";

/********** PINS **********/
#define SOIL1_PIN 32
#define SOIL2_PIN 33

#define RELAY1_PIN 26
#define RELAY2_PIN 27

#define DHT_PIN   4
#define DHTTYPE  DHT11
DHT dht(DHT_PIN, DHTTYPE);

/********** VARIABLES **********/
bool pump1On = false;
bool pump2On = false;
bool autoMode = true;

unsigned long lastReadTime = 0;

/********** CLIENTS **********/
WiFiClient espClient;
PubSubClient mqttClient(espClient);

/****************************************************
 * SETUP
 ****************************************************/
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("AgriConnect ESP32 Booting...");

  pinMode(SOIL1_PIN, INPUT);
  pinMode(SOIL2_PIN, INPUT);

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);

  // Pumps OFF at boot (ACTIVE-LOW relay)
  digitalWrite(RELAY1_PIN, HIGH);
  digitalWrite(RELAY2_PIN, HIGH);

  dht.begin();

  /******** WIFI CONNECT ********/
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n WiFi Connected");
  Serial.print("📡 ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  /******** MQTT SETUP ********/
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

/****************************************************
 * LOOP
 ****************************************************/
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastReadTime >= READ_INTERVAL_MS) {
    lastReadTime = now;

    int soil1 = digitalRead(SOIL1_PIN); // HIGH=DRY, LOW=WET
    int soil2 = digitalRead(SOIL2_PIN);

    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();

/******** AUTO MODE ********/
if (autoMode) {
  // Sensor: HIGH = WET, LOW = DRY
  // Relay : LOW  = ON , HIGH = OFF

  pump1On = (soil1 == LOW);   // DRY → pump ON
  pump2On = (soil2 == LOW);   // DRY → pump ON
}

    /******** RELAY CONTROL (ACTIVE-LOW) ********/
    digitalWrite(RELAY1_PIN, pump1On ? LOW : HIGH);
    digitalWrite(RELAY2_PIN, pump2On ? LOW : HIGH);

    /******** MQTT PUBLISH ********/
    mqttClient.publish(TOPIC_SOIL1, soil1 == HIGH ? "DRY" : "WET", true);
    mqttClient.publish(TOPIC_SOIL2, soil2 == HIGH ? "DRY" : "WET", true);

    mqttClient.publish(TOPIC_PUMP1_STATUS, pump1On ? "ON" : "OFF", true);
    mqttClient.publish(TOPIC_PUMP2_STATUS, pump2On ? "ON" : "OFF", true);

    mqttClient.publish(TOPIC_TEMP, String(temp).c_str(), true);
    mqttClient.publish(TOPIC_HUM, String(hum).c_str(), true);

    sendToThingSpeak(soil1, soil2, temp, hum, pump1On, pump2On);

    /******** SERIAL DEBUG ********/
    Serial.println("\n📤 MQTT Data Published:");
    Serial.print("   Soil1: ");
    Serial.print(soil1 == HIGH ? "DRY" : "WET");
    Serial.print(" | Pump1: ");
    Serial.println(pump1On ? "ON" : "OFF");

    Serial.print("   Soil2: ");
    Serial.print(soil2 == HIGH ? "DRY" : "WET");
    Serial.print(" | Pump2: ");
    Serial.println(pump2On ? "ON" : "OFF");

    Serial.print("   Temperature: ");
    Serial.print(temp);
    Serial.print(" °C | Humidity: ");
    Serial.print(hum);
    Serial.println(" %");
  }
}

/****************************************************
 * MQTT CALLBACK
 ****************************************************/
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];
  msg.toUpperCase();

  Serial.print("📥 MQTT Message [");
  Serial.print(topic);
  Serial.print("] : ");
  Serial.println(msg);

  if (String(topic) == TOPIC_PUMP1_CTRL) {
    if (msg == "ON")   { pump1On = true;  autoMode = false; }
    if (msg == "OFF")  { pump1On = false; autoMode = false; }
    if (msg == "AUTO") autoMode = true;
  }

  if (String(topic) == TOPIC_PUMP2_CTRL) {
    if (msg == "ON")   { pump2On = true;  autoMode = false; }
    if (msg == "OFF")  { pump2On = false; autoMode = false; }
    if (msg == "AUTO") autoMode = true;
  }
}

/****************************************************
 * MQTT RECONNECT
 ****************************************************/
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print(" Connecting to MQTT Broker... ");

    if (mqttClient.connect("AgriESP32")) {
      Serial.println("CONNECTED");
      mqttClient.subscribe(TOPIC_PUMP1_CTRL);
      mqttClient.subscribe(TOPIC_PUMP2_CTRL);
      Serial.println("Subscribed to pump control topics");
    } else {
      Serial.print("FAILED (rc=");
      Serial.print(mqttClient.state());
      Serial.println(") retrying in 3 seconds");
      delay(3000);
    }
  }
}

/****************************************************
 * THINGSPEAK UPDATE
 ****************************************************/
void sendToThingSpeak(int s1, int s2, float t, float h, bool p1, bool p2) {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(THINGSPEAK_SERVER) + "/update?api_key=" + thingspeakApiKey +
               "&field1=" + String(s1 == HIGH ? 1 : 0) +
               "&field2=" + String(s2 == HIGH ? 1 : 0) +
               "&field3=" + String(t) +
               "&field4=" + String(h) +
               "&field5=" + String(p1 ? 1 : 0) +
               "&field6=" + String(p2 ? 1 : 0);

  http.begin(url);
  http.GET();
  http.end();
}
