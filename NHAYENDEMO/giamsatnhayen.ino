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


/*======================== Defination of PIN ================================*/
// D1 is output of reading temperature and humidity
#define DHTPIN D1


/*************************** Sketch Code ************************************/

// Replace with your network credentials
const char* ssid     = "Le An";
const char* password = "hohoa46491990";
 
// REPLACE with your Domain name and URL path or IP address with path
const char* serverName = "http://192.168.1.8/esp_data.php";
 
//const int oneWireBus = 4;
// Keep this API Key value to be compatible with the PHP code provided in the project page. 
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key 
String apiKeyValue = "kjsjkhjdhfd";
 
String sensorName = "DHT11";
String sensorLocation = "home";
float temperatureC = 0.0;
float humidity = 0.0;  
DHT dht(DHTPIN, DHT11);



/************************* Adafruit.io Setup *********************************/

#define SERVER    "http://localhost/" // Server was written by php
#define PORT      1883
#define USERNAME  "root"
#define PASSWORD  ""
//#define DEVICE_ID "Xxs2h2"

/************ Global State (you don't need to change this!) ******************/

// Create a BridgeClient instance to communicate using the Yun's bridge & Linux OS.
BridgeClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, SERVER, PORT, USERNAME, PASSWORD);


/****************************** Feeds ***************************************/

// Setup a feed called 'assetPub' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish assetPub = Adafruit_MQTT_Publish(&mqtt, "up/client/" USERNAME "/asset/" DEVICE_ID);

// Setup a feed called 'assetSub' for subscribing to changes.
Adafruit_MQTT_Subscribe assetSub = Adafruit_MQTT_Subscribe(&mqtt, "down/client/" USERNAME "/asset/" DEVICE_ID);


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

int LIGHTPIN = 15; //D8
String sPayload;
char* cPayload;

void setup() {

  /* =================== MQTT ============*/
  Bridge.begin();
  Console.begin();
  
  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&assetSub);

  // initialize digital pin LED_BUILTIN as an output.
  //pinMode(LED_BUILTIN, OUTPUT);




  /* ========== HTTP ===========*/
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
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

  pinMode(LIGHTPIN, OUTPUT); // D8 is output pin}
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
    delay(60000);  // 60s
  }
  else {
    Serial.println("WiFi Disconnected");
    digitalWrite(2, HIGH);
    delay(250);
    Serial.print(".");
    digitalWrite(2, LOW); 
    delay(250);
  }

  /*=========== MQTT ==========*/
  // lack of somethings - a broker so we could not use this one
  MQTT_connect();
  
  /* ======= PUBLISHING ====== */
  /*
  analogReference(EXTERNAL);
  pinMode(12,OUTPUT);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& payload = jsonBuffer.createObject();
  payload["device_id"] = DEVICE_ID;
  payload["lightlevel"] = getTemperature();
  
  
  sPayload = "";
  payload.printTo(sPayload);
  cPayload = &sPayload[0u];
  
  // Now we can publish stuff!
  Console.print(F("\nPublishing "));
  Console.print(cPayload);
  Console.print("...");
  
  if (! assetPub.publish(cPayload)) {
  Console.println(F("Failed"));
  } else {
  Console.println(F("OK!"));
  }
  */

  /* ============ Subcriber ==============*/
  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(300))) {
    if (subscription == &assetSub) {
     Console.print(F("Got: "));
     Console.println((char *)assetSub.lastread);
    
     if (strcmp((char *)assetSub.lastread, "ON") == 0) {
       digitalWrite(LIGHTPIN, HIGH);
     }
     if (strcmp((char *)assetSub.lastread, "OFF") == 0) {
       digitalWrite(LIGHTPIN, LOW);
     }
    }
  }
  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
  Console.println(F("MQTT Ping failed."));
  }
  delay(9000);  // wait 9 seconds
}

void MQTT_connect() {
 int8_t ret;

 // Stop if already connected.
 if (mqtt.connected()) {
   return;
 }

 Console.print("Connecting to MQTT... ");

 while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
      Console.println(mqtt.connectErrorString(ret));
      Console.println("Retrying MQTT connection in 5 seconds...");
      mqtt.disconnect();
      delay(5000);  // wait 5 seconds
 }
 Console.println("MQTT Connected!");
}
