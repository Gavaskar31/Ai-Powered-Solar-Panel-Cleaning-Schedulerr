#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

const char* ssid = "TEMP";
const char* password = "temp12345";

unsigned long channelID = 3302015;
const char* writeAPIKey = "7W37YMNBFYQUJGRO";

WiFiClient client;

#define CURRENT_SENSOR A0
#define RELAY D5
#define BUTTON D6

float currentValue = 0;
float thresholdCurrent = 0.28;  // 280 mA

bool cleaning = false;
bool cleanedToday = false;

unsigned long lastUpdate = 0;

void setup()
{
Serial.begin(115200);

pinMode(RELAY, OUTPUT);
pinMode(BUTTON, INPUT_PULLUP);

digitalWrite(RELAY, LOW);

Wire.begin(D2, D1);
rtc.begin();

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED)
{
delay(500);
Serial.print(".");
}

Serial.println("WiFi Connected");

ThingSpeak.begin(client);
}

void loop()
{
DateTime now = rtc.now();

// Read current sensor
int sensorValue = analogRead(CURRENT_SENSOR);
float voltage = sensorValue * (3.3 / 1023.0);

// ⚠ Adjust this calibration if needed
currentValue = voltage / 0.1;

Serial.print("Current: ");
Serial.println(currentValue);

// ✅ Daily scheduled cleaning at 10:00 AM
if(now.hour() == 10 && now.minute() == 0 && cleanedToday == false)
{
Serial.println("Scheduled Cleaning");
startCleaning();
cleanedToday = true;
}

// Reset at midnight
if(now.hour() == 0 && now.minute() == 0)
{
cleanedToday = false;
}

// ✅ Dust detection using fixed threshold
if(currentValue < thresholdCurrent && cleaning == false)
{
Serial.println("Low Current - Dust Detected");
startCleaning();
}

// ✅ Manual cleaning
if(digitalRead(BUTTON) == LOW)
{
Serial.println("Manual Cleaning");
startCleaning();
delay(500);
}

// ✅ Send to ThingSpeak
if(millis() - lastUpdate > 20000)
{
ThingSpeak.setField(1, currentValue);

if(cleaning)
ThingSpeak.setField(2, 1);
else
ThingSpeak.setField(2, 0);

ThingSpeak.writeFields(channelID, writeAPIKey);

lastUpdate = millis();
}

delay(2000);
}

void startCleaning()
{
cleaning = true;

digitalWrite(RELAY, HIGH);

delay(15000);

digitalWrite(RELAY, LOW);

cleaning = false;

Serial.println("Cleaning Done");
}