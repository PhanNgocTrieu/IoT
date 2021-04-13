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

/* ===================== MQTT's variables ===========================*/
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

#define WIFI_SSID "Le An"
#define WIFI_PASSWORD "hohoa46491990"


#define MQTT_HOST IPAddress(192, 168, 1, 8) // our ID address
#define MQTT_PORT 1883

#define PIN_PUMP D8

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
  uint16_t packetIdSub = mqttClient.subscribe("D8/on", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);

  // publish
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
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: "); 
  Serial.println(packetId);
  digitalWrite(PIN_PUMP,HIGH);
  delay(1000);
  digitalWrite(PIN_PUMP,LOW);
}

/* ==================== END OF MQTT's Declaration =============================*/



/*======================== Defination of PIN ================================*/
// D1 is output of reading temperature and humidity
#define DHTPIN D1


/*************************** Sketch Code ************************************/

// Replace with your network credentials
const char* ssid     = "Le An"; // "Le An"
const char* password = "hohoa46491990"; // "hohoa46491990"
 
// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://192.168.1.8/esp_data.php"; // checking IP - cmd -> ipconfig => when changing wifi -> Ip would be change
 
//const int oneWireBus = 4;
// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "kjsjkhjdhfd";
 
String sensorName = "DHT11";
String sensorLocation = "home";
float temperatureC = 0.0;
float humidity = 0.0;  
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
  mqttClient.onConnect(onMqttConnect);
  //mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish); // Call Publish function
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  
  /* ========== MQTT ===========*/
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  /* ========== HTTP ===========*/
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(PIN_PUMP,OUTPUT);
  digitalWrite(16, LOW); // chưa kết nối;
   
  Serial.begin(115200);


  // Start the DHT sensor
  dht.begin();
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  
  while(WiFi.status() != WL_CONNECTED) { 
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
  if(WiFi.status()== WL_CONNECTED){
    digitalWrite(16, HIGH);  // đã kết nối
    HTTPClient http;

    http.begin(serverName);
    //sensors.requestTemperatures();
    
    humidity = getHumidity();
    
    // Read temperature as Celsius (the default)
    temperatureC = getTemperature();
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Prepare your HTTP POST request data
    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName + "&location=" + sensorLocation + "&temp=" + temperatureC + "&hum="+ humidity;
    
    Serial.print("httpRequestData: ");
    Serial.println(httpRequestData);
    
    // Send HTTP POST request
    
    int httpResponseCode = http.POST(httpRequestData);
    
    String payload = http.getString();
        
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      Serial.println(payload);  
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
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
}
