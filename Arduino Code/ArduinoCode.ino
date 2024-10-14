#include <SoftwareSerial.h>

#define SMOKE_SENSOR_PIN A0  // Smoke sensor connected to A0
#define TOUCH_SENSOR_PIN A1  // Touch sensor connected to A1

SoftwareSerial XBee(2, 3);  // RX, TX for XBee communication

// Threshold to determine if smoke level is high
const int SMOKE_THRESHOLD = 300;  // Adjust this based on your environment

void setup() {
  Serial.begin(9600);       // Serial for debugging
  XBee.begin(9600);         // XBee communication
  
  pinMode(SMOKE_SENSOR_PIN, INPUT);
  pinMode(TOUCH_SENSOR_PIN, INPUT);
  
  // Only print the "System Initialized" message once
  Serial.println("System Initialized");
  XBee.println("System Initialized");
}

void loop() {
  int smokeValue = analogRead(SMOKE_SENSOR_PIN);
  int touchValue = digitalRead(TOUCH_SENSOR_PIN);

  // Prepare the data string to send over XBee
  String dataToSend = "SMOKE:" + String(smokeValue) + ",TOUCH:" + String(touchValue);
  
  // Print the sensor values to both the Arduino serial monitor and XBee
  Serial.println(dataToSend);
  XBee.println(dataToSend);  // Send data over XBee

  // Check if there are any incoming commands from XBee (if required)
  if (XBee.available()) {
    String command = XBee.readStringUntil('\n');
    command.trim();
    
    // In case you want to add command handling in the future, this section is kept
    // For now, there is no specific command handling for the touch/smoke system.
  }

  delay(5000);  // Adjust the delay for sensor reading intervals
}
