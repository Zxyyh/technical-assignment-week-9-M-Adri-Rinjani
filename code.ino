#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

#define DHTTYPE DHT11

const char* ssid = "ssid";
const char* password = "password";
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char* mqtt_server = "192.168.1.8";

WiFiClient espClient;
PubSubClient client(espClient);

const int DHTPin = 5;


DHT dht(DHTPin, DHTTYPE);

long now = millis();
long lastMeasure = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();


}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void setup() {
  
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void loop() {

  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float f = dht.readTemperature(true);

    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    

    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    client.publish("esp/temperature", temperatureTemp);
    client.publish("esp/humidity", humidityTemp);
    lcd.setCursor(0,0);
    lcd.print("Temp F= ");
    lcd.print(temperatureTemp);
    lcd.setCursor(0,1);
    lcd.print("Humidity= ");
    lcd.print(humidityTemp);
    lcd.print(" %");
    delay(1000);
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");

  }
} 
