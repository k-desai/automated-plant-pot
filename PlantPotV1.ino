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
char* Version ="V1.1";

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////
#define OLED_RESET LED_BUILTIN  //4
#define ONE_WIRE_BUS D3
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
char* ExtraMessage;
int sensorValue = 0;
String temperatureString;
int BootCount = 0;
unsigned long lastInterrupt;

//  DS18B20 Setup

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING );
  pinMode(MoisturePower, OUTPUT);     // Assigning pin as output
  pinMode(PumpPower, OUTPUT);     // Assigning pin as output
  display.clearDisplay();
  display.display();
  WiFi.mode(WIFI_OFF);
  DS18B20.begin();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main functions being called.
void CheckAndWater(){
      noInterrupts();
      ReadMoistureLevel();
      MoistureLogic();
      RequestTemperature();
      DisplayStats();
      OneLine_Text(ExtraMessage);
      WaterPlant();
      //ClearDisplay();
      interrupts(); 
}

//  Display on OLED Screen.
void DisplayStats(){
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,5);
        display.print("Moisture:");
        display.print(moisture_percentage);
        display.println("%");
        display.setCursor(0,20);
        display.print("Temp:");
        display.print(temperatureString);
        display.print((char)247);
        display.println("C");
        display.display();
        delay(5000);
        ClearDisplay();
     }
void Boot_Text(){
    if (BootCount == 0)
  {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,5);
        display.println("Automated Plant Pot");
        display.println("  KetanDesai.co.uk");
        display.print("   Version :");
        display.print(Version);
        display.display();
        delay(5000);
        ClearDisplay();
        BootCount = 1;                            
  }
     }     
void OneLine_Text(char *ThisString){
        display.setCursor(0,15);
        display.setTextSize(1);
        display.println(ThisString);
        display.display();
        delay(3000);
        ClearDisplay();
}
     
// Clear the display of any information.
void ClearDisplay(){
        display.clearDisplay();
        display.display();
}
// What to do when an interrupt is detected
void handleInterrupt() {
  if(millis() - lastInterrupt > 5000) // we set a 10ms no-interrupts window
    {    
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,15);
        display.println("Button pushed :)");
        display.display();
        delay(5000);
        ClearDisplay();
    //CheckAndWater();
     lastInterrupt = millis();
    }   
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
  ExtraMessage = "No Watering Required";
  Waterflag = false;
  }
}
// activate the motor to water the pot
void WaterPlant(){
  if (Waterflag){
  digitalWrite(PumpPower, HIGH);    // making pin low
  OneLine_Text("Watering for 5s...");
  delay(5000);
  digitalWrite(PumpPower, LOW);    // making pin low
  Waterflag = false;
    }
      else
        {
         OneLine_Text("Sleeping...");
          delay(5000);
        }
  }
void RequestTemperature() 
  {
    DS18B20.requestTemperatures();
    temperatureString = String (DS18B20.getTempCByIndex(0));
  }
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Void Loop
///////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
      //WiFi.forceSleepBegin();
      Boot_Text();
      CheckAndWater();
      delay(3600000); // Check every 1 hour from last watering.
}
