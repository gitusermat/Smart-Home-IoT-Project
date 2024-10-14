#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi and MQTT configuration
const char* ssid = "Joy";
const char* password = "1234567890";
const char* mqtt_server = "192.168.48.91";
const int mqtt_port = 1883;

// DHT22 settings
#define DHTPIN D4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// LDR settings
#define LDR_PIN A0

// Bulb and Buzzer settings
#define BULB_PIN D2
#define BUZZER_PIN D5

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastSensorReadTime = 0;
const unsigned long sensorInterval = 2000;  // 2 seconds

void setup_wifi() {
  Serial.begin(115200);
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(msg);

  if (String(topic) == "BulbControl") {
    if (msg == "ON") {
      digitalWrite(BULB_PIN, HIGH);
      Serial.println("Bulb turned ON");
    } else if (msg == "OFF") {
      digitalWrite(BULB_PIN, LOW);
      Serial.println("Bulb turned OFF");
    }
  }

  if (String(topic) == "BuzzerControl") {
    if (msg == "ON") {
      digitalWrite(BUZZER_PIN, LOW);
      Serial.println("Buzzer turned ON");
    } else if (msg == "OFF") {
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("Buzzer turned OFF");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      client.subscribe("BulbControl");
      client.subscribe("BuzzerControl");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  dht.begin();
  
  pinMode(BULB_PIN, OUTPUT);
  digitalWrite(BULB_PIN, LOW);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println("Setup complete");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentTime = millis();
  if (currentTime - lastSensorReadTime > sensorInterval) {
    lastSensorReadTime = currentTime;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int lightLevel = analogRead(LDR_PIN);

    if (!isnan(humidity) && !isnan(temperature)) {
      String payload = "{\"temperature\":";
      payload += String(temperature);
      payload += ",\"humidity\":";
      payload += String(humidity);
      payload += ",\"light_level\":";
      payload += String(lightLevel);
      payload += "}";

      client.publish("sensor_data", payload.c_str(), true);

      // Display sensor values on Serial Monitor
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print(" C, Humidity: ");
      Serial.print(humidity);
      Serial.print(" %, Light Level: ");
      Serial.println(lightLevel);
    }
  }
}
