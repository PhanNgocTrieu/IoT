#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif
 
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
 
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

DHT dht(D1, DHT11);

//OneWire oneWire(oneWireBus);
//DallasTemperature sensors(&oneWire);

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
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(16, LOW); // chưa kết nối
  Serial.begin(115200);

  // Start the DHT sensor
  dht.begin();
  
  // Start the DS18B20 sensor
  //sensors.begin();
  
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
    
    //sensors.requestTemperatures();
    
    humidity = getHumidity();
    
    // Read temperature as Celsius (the default)
    temperatureC = getTemperature();
    
    http.begin();
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
    delay(6000); 
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
