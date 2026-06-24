// Blynk app token
#define BLYNK_TEMPLATE_ID "TMPL31D20W7J3"
#define BLYNK_TEMPLATE_NAME "Home Automation IoT Project"
#define BLYNK_AUTH_TOKEN "xyz"
// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#include "DHTesp.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

BlynkTimer timer;
#define DHTPIN 27   // DHT 22  (AM2302), AM2321

const int motion_sensor_pin = 15;
const int flame_sensor_pin = 4;
const int gas_sensor_pin = 36;
const int water_sensor_pin = 39;
const int pump_pin = 2;
const int buzzer_pin = 14;

DHTesp dht;

const int DELAY_UPDATE = 1000;
uint32_t timerUpdate = 0;
int gas_sensor_value = 0;
int gas_output_value = 0;
int water_sensor_value = 0;
int water_output_value = 0;

int pirState = LOW;
int val = 0;

//Change the virtual pins according the rooms
#define gas_sensor_vpin V1
#define water_sensor_vpin V2
#define flame_sensor_vpin V3
#define motion_sensor_vpin V4
#define temperature_sensor_vpin V5
#define pump_vpin V7

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED() {
  Blynk.syncVirtual(gas_sensor_vpin);
  Blynk.syncVirtual(water_sensor_vpin);
  Blynk.syncVirtual(flame_sensor_vpin);
  Blynk.syncVirtual(motion_sensor_vpin);
  Blynk.syncVirtual(temperature_sensor_vpin);
  Blynk.syncVirtual(pump_vpin);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  pinMode(motion_sensor_pin, INPUT);
  pinMode(gas_sensor_pin, INPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(flame_sensor_pin, INPUT);
  pinMode(water_sensor_pin, INPUT);

  pinMode(pump_pin, OUTPUT);
  pinMode(buzzer_pin, OUTPUT);

  dht.setup(DHTPIN, DHTesp::DHT22);
  Blynk.begin(auth, ssid, pass);

}

void loop()
{
  // blynk run
  Blynk.run();
  timer.run();

  // Reading gas sensor value
  gas_sensor_value = analogRead(gas_sensor_pin);
  gas_output_value = map(gas_sensor_value, 0, 4095, 0, 255);
  //Serial.print("Gas: ");
  //Serial.println(gas_output_value);
  delay(100);
  if (gas_output_value > 165) {
    Serial.println("Gas Leakge Detected!");
    Blynk.logEvent("warning_message", "Gas Leakage Detected!");
     // send status to Blynk
    Blynk.virtualWrite(gas_sensor_vpin, 1);
     Blynk.virtualWrite(pump_vpin, 1);
     while(gas_output_value > 165){
       digitalWrite(pump_pin, HIGH);
       gas_sensor_value = analogRead(gas_sensor_pin);
        gas_output_value = map(gas_sensor_value, 0, 4095, 0, 255);
     }

   
  }
  else {
    digitalWrite(pump_pin, LOW);
    Blynk.virtualWrite(pump_vpin, 0);
     // send status to Blynk
    Blynk.virtualWrite(gas_sensor_vpin, 0);
  }

  // Reading water sensor value
  water_sensor_value = analogRead(water_sensor_pin);
  water_output_value = map(water_sensor_value, 0, 4095, 0, 255);
  //Serial.print("Water: ");
  //Serial.println(water_output_value);
  //Blynk.virtualWrite(water_sensor_vpin, water_output_value);
  delay(100);
  if (water_output_value < 20) {
    // send a message warning!
    Serial.println("Water Over!");
    Blynk.logEvent("warning_message", "Water Level LOW!");
     // send status to Blynk
    Blynk.virtualWrite(water_sensor_vpin, 1);
    delay(500);
    
  }
  else{
     // send status to Blynk
    Blynk.virtualWrite(water_sensor_vpin, 0);
  }
  temperatureSensor();
  motionSensor();
  flameSensor();
}


void temperatureSensor() {
  if (millis() - timerUpdate >= DELAY_UPDATE) {
    timerUpdate = millis();
    TempAndHumidity  data = dht.getTempAndHumidity();
    Blynk.virtualWrite(temperature_sensor_vpin, data.temperature);
    if (data.temperature >= 50) {
      Serial.println("Over Temperature");
      Blynk.logEvent("warning_message", "Over Temperature Detected!");
      delay(100);
      while(data.temperature > 50){
        digitalWrite(pump_pin, HIGH);
         Blynk.virtualWrite(pump_vpin, 1);
        data = dht.getTempAndHumidity();
      }
    }
  }
}

void motionSensor() {
  val = digitalRead(motion_sensor_pin);  // read input value
  if (val == HIGH) {            // check if the input is HIGH
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      Blynk.logEvent("warning_message", "Someone might be inside the home!");
       // send status to Blynk
    Blynk.virtualWrite(motion_sensor_vpin, 1);
      pirState = HIGH;
    }
  }
  else {
    // send message motion end
    if (pirState == HIGH) {
      Serial.println("Motion ended!");
       // send status to Blynk
    Blynk.virtualWrite(motion_sensor_vpin, 0);
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
}

void flameSensor() {
  if (digitalRead(flame_sensor_pin) == HIGH) {
    digitalWrite(pump_pin, HIGH);
     Blynk.virtualWrite(pump_vpin, 1);
     // send status to Blynk
    Blynk.virtualWrite(flame_sensor_vpin, 1);
    Blynk.logEvent("warning_message", "Fire Detected!");
    while(digitalRead(flame_sensor_pin) != LOW){
      tone(buzzer_pin, 550);
      delay(500);
      tone(buzzer_pin, 0);
      delay(500);
    }
  }
  else{
     // send status to Blynk
    Blynk.virtualWrite(pump_vpin, 0);
    Blynk.virtualWrite(flame_sensor_vpin, 0);
  }
  delay(100);
}
