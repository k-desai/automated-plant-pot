///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

const byte interruptPin = D4;
volatile byte interruptCounter = 0;
int numberOfInterrupts = 0;
volatile bool Waterflag = false;
const int sensor_pin = A0;  /* Connect Soil moisture analog sensor pin to A0 of NodeMCU */
const int MoisturePower = D5;  // Assigning name to Trasistor 
const int PumpPower = D6;  // Assigning name to Trasistor 
float moisture_percentage;
int WaterAtLevel = 50; //0 - 100 Percentage level
String ExtraMessage;
int sensorValue = 0; 

//  DS18B20 Setup

#define ONE_WIRE_BUS D4 //Pin to which is attached a temperature sensor
#define ONE_WIRE_MAX_DEV 15 //The maximum number of devices

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING );
  pinMode(MoisturePower, OUTPUT);     // Assigning pin as output
  pinMode(PumpPower, OUTPUT);     // Assigning pin as output
  display.clearDisplay();
  display.display();
  WiFi.mode(WIFI_OFF);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main functions being called.
void CheckAndWater(){
      ReadMoistureLevel();
      MoistureLogic();
      DisplayStats();
      WaterPlant();
      ClearDisplay();
}

//Display on OLED Screen.
void DisplayStats(){
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.print("Soil Moisture= ");
        display.print(moisture_percentage);
        display.println("%");
        display.println(ExtraMessage);
        display.display();
     }
// Clear the display of any information.
void ClearDisplay(){
        display.clearDisplay();
        display.display();
}
// What to do when an interrupt is detected
void handleInterrupt() {
    CheckAndWater();
}
//Read the Moisture level and format it
void ReadMoistureLevel(){
  digitalWrite(MoisturePower, HIGH);   // making pin high
  delay(1000);
  sensorValue = analogRead(sensor_pin);
  moisture_percentage = convertToPercent(sensorValue);
  delay(1000);
  digitalWrite(MoisturePower, LOW);    // making pin low
}
// Convert the value from the moisture sensor to a value between 0-100
int convertToPercent(int value){
  int percentValue = 0;
  percentValue = map(value, 1023, 465, 0, 100);
  return percentValue;
}
// Do action on moisture level
void MoistureLogic(){
  if (moisture_percentage < WaterAtLevel)
  {
  ExtraMessage = "Needs Watering";
  Waterflag = true;                             
  }
  else
  {
  ExtraMessage = "Does not need Watering";
  Waterflag = false;
  }
}
// activate the motor to water the pot
void WaterPlant(){
  if (Waterflag){
  digitalWrite(PumpPower, HIGH);    // making pin low
  display.setCursor(0,20);
  display.print("Watering for 5s...");
  display.display();
  delay(5000);
  digitalWrite(PumpPower, LOW);    // making pin low
  Waterflag = false;
  }else{
  display.setCursor(0,20);
  display.print("Sleeping...");
  display.display();
  delay(5000);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Void Loop
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
WiFi.forceSleepBegin();
      CheckAndWater();
      delay(3600000); // Check every 1 hour from last watering.
}
