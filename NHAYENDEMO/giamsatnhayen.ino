#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#endif

/* Libraries for MQTT */
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Bridge.h>
#include <Console.h>
#include <BridgeClient.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>



/* ============== DEFINE WIFI =============*/
#define WIFI_SSID "Le An" // "Le An"
#define WIFI_PASSWORD "hohoa46491990" 

//#define WIFI_SSID "CAO COFFEE & OFFICE" // "Le An"
//#define WIFI_PASSWORD "68686868" 

/* ============== DEFINE IPADDRESS WHICH CONNECTED TO WIFI ===========*/
// for using MQTT
//#define MQTT_HOST IPAddress(192,168,1,211) // // 192.168.1.211 -- CAO Cofffe
#define MQTT_HOST IPAddress(192,168,1,8) //LE AN


#define MQTT_PORT 1883


// Temperature MQTT Topics
#define MQTT_PUB_TEMP "esp/trieu/temperature"
#define MQTT_PUB_HUM  "esp/trieu/humidity"

// Subscribe topic:
#define MQTT_SUB_WATERPUMP "esp/trieu/D8"
#define MQTT_SUB_SPINNING "esp/trieu/D3"

#define MQTT_SUB_AUTO "esp/trieu/D7" // auto_custom


AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

Ticker wifiReconnectTimer;


/*======================== Defination of PIN ================================*/
// D1 is output of reading temperature and humidity
#define DHTPIN D1
#define WATER_PUMPER_PIN 15
#define AUTO_CUSTOM_PIN 13
#define SPINNING_PIN 0


/*======================== Defination of SOMETHINGS =========================*/

String sensorName = "DHT11";
String sensorLocation = "home";
float temperature = 0.0;
float humidity = 0.0;



/* new*/
String convertToString(char* a, int size)
{
    int i;
    String s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

/* ====================== MQTT's functions ==========================*/
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

 // Subscribe
 uint16_t packetIdSub = mqttClient.subscribe("test/lol", 2); // nhận => subcribe
 Serial.print("Subscribing at QoS 2, packetId: ");
 Serial.println(packetIdSub);

  publish
   mqttClient.publish("test/lol", 0, true, "test 1");
   Serial.println("Publishing at QoS 0");
   uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
   Serial.print("Publishing at QoS 1, packetId: ");
   Serial.println(packetIdPub1);
   uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
   Serial.print("Publishing at QoS 2, packetId: ");
   Serial.println(packetIdPub2);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.printf("  payload: %s \n",String(payload).c_str());


  // AUTO PIN
  if (strcmp(topic,MQTT_SUB_AUTO) == 0)
  {
    Serial.printf("This is running in MQTT_SUB_AUTO \n");
    if (strncmp((char* )payload,"on",2) == 0)
    {
      Serial.printf("Turn On D7\n");
      digitalWrite(AUTO_CUSTOM_PIN, HIGH);
    }
    if (strncmp((char*)payload,"off",3) == 0)
    {
      Serial.printf("Turn Off D7\n");
      digitalWrite(AUTO_CUSTOM_PIN, LOW);
    }
  }


  // IN AUTO MODE
  if (digitalRead(AUTO_CUSTOM_PIN) == 1)
  {
    int value = digitalRead(AUTO_CUSTOM_PIN);
    Serial.println(value);
    
    // WATER PUMPER
    if (strcmp(topic,MQTT_SUB_WATERPUMP) == 0)
    {
      Serial.printf("This is running in MQTT_SUB_WATERPUMP \n");
      if (strncmp((char* )payload,"on",2) == 0)
      {
        Serial.printf("Turn On D8\n");
        digitalWrite(WATER_PUMPER_PIN, HIGH);
      }
      if (strncmp((char* )payload,"off",3) == 0)
      {
        Serial.printf("Turn Off D8\n");
        digitalWrite(WATER_PUMPER_PIN, LOW);
      }
    }


    // FAN
    if (strcmp(topic,MQTT_SUB_SPINNING) == 0)
    {
      Serial.printf("This is running in SPINNING \n");
      if (strncmp((char* )payload,"on",2) == 0)
      {
        Serial.printf("Turn On D3\n");
        digitalWrite(SPINNING_PIN, HIGH);
      }
      if (strncmp((char* )payload,"off",3) == 0)
      {
        Serial.printf("Turn Off D3\n");
        digitalWrite(SPINNING_PIN, LOW);
      }
    }
  }

  
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/* ==================== END OF MQTT's Declaration =============================*/



/* ====== Starting DHT =========*/
DHT dht(DHTPIN, DHT11);


float getTemperature()
{
  float temp = dht.readTemperature();
  if (isnan(temp))
    Serial.println("reading temp failed");
  return temp;
}

float getHumidity()
{
  float hum = dht.readHumidity();
  if (isnan(hum))
    Serial.println("reading hum failed");

  return hum;
}

void setup() {
  /* ========== MQTT ===========*/
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  /* ========== MQTT ===========*/
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish); 
  mqttClient.setServer(MQTT_HOST, MQTT_PORT); 


  // connecting wifi pin:
  pinMode(2, OUTPUT);


  // WATER_PUMPER_PIN:
  pinMode(WATER_PUMPER_PIN, OUTPUT);
  pinMode(AUTO_CUSTOM_PIN, OUTPUT);
  pinMode(SPINNING_PIN, OUTPUT);



  digitalWrite(16, LOW); // chưa kết nối

  Serial.begin(115200);


  // Start the DHT sensor
  dht.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(2, HIGH);
    delay(250);
    Serial.println(".");
    digitalWrite(2, LOW);
    delay(250);
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());


}

void loop() {


  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    humidity = getHumidity();

    // Read temperature as Celsius (the default)
    temperature = getTemperature();

    //Send an HTTP POST request every 6 seconds
    delay(6000);  // 60s
  }
  else {
    Serial.println("WiFi Disconnected");
    digitalWrite(2, HIGH);
    delay(250);
    Serial.print(".");
    digitalWrite(2, LOW);
    delay(250);
  }

  // Subscribe an MQTT message on topic esp/trieu/D8
  // nhận => subcribe
  uint16_t packetIdSub1 = mqttClient.subscribe(MQTT_SUB_AUTO, 1);
  uint16_t packetIdSub2 = mqttClient.subscribe(MQTT_SUB_WATERPUMP, 1);
  uint16_t packetIdSub3 = mqttClient.subscribe(MQTT_SUB_SPINNING, 1);
  

  
   // Publish an MQTT message on topic esp/trieu/temperature
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temperature).c_str());
  Serial.printf("Publishing on topic %s at QoS 1, packetId: %i", MQTT_PUB_TEMP, packetIdPub1);
  Serial.printf("Message: %.2f \n", temperature);

  // Publish an MQTT message on topic esp/trieu/humidity
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(humidity).c_str());
  Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_HUM, packetIdPub2);
  Serial.printf("Message: %.2f \n", humidity);

  
  
 
}
