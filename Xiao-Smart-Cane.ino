// Smart Cane
// by Sean Hopkins

// Demonstrates use of the Wire library
// 
//

// Created Oct. 2021
#include <NMEAGPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>                             
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MLX90614.h>
#include <MPU6050_light.h>
#include <DHT.h>
#include <DHT_U.h>
#include <GPSport.h>


#define DHTPIN 1
#define DHTTYPE DHT11
////////////////////////////////////////////////////////////////////////////////////////////
//Objects
//
//ALL Objects used in the project
//
//
////////////////////////////////////////////////////////////////////////////////////////////
NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values
MAX30105 pulseOxymeter;
RTC_DS3231 rtc;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
MPU6050 mpu(Wire);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
DHT_Unified dht(DHTPIN, DHTTYPE);
SoftwareSerial BT(2,3); //RX|TX
////////////////////////////////////////////////////////////////////////////////////////////
//Variables
//
//ALL global variables used in the project
//
//
////////////////////////////////////////////////////////////////////////////////////////////
char daysOfTheWeek[7][12] = {"Sun.", "Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat."};
const int MODEBUTTONPIN = 8, SELECTBUTTONPIN = 9,BUZZERPIN = 10,PHOTOSENSORPIN = 0,
          buttonDelay = 150, DELAY = 2000, PITCH = 3000,
          irMode = 1, heartMode = 2, timeMode = 0, conMode = 3, numModes=4;
int buttonState = 0, currentMode = 0, currentSelect = 0, iter = 0, msgNum = 0,
    accelTimer = 0, RTCTimer = 0, pulseOxyTimer = 0, irTimer = 0, dhtTimer = 0, displayTimer = 0, fallIter = 0,
    lightTimer = 0, gameTimer = 0, gpsTimer = 0,btTimer0 = 0, btTimer1 = 0, fallTimer = 0, buzzTimer = 0, gpsDisplayTimer = 0;
float avg_temp = 0, final_temp = 0, beatsPerMinute, beatAvg, ballX = 0, ballY= 0;
boolean finished = false, isRunning = false, lightWarning = false, fallWarning = false, freeFall = false,
        sleepLight = false, sleepFall = false, night = false, gpsSend = false, heartFull = false, gpsData = false;
double lat = 0.00, lon = 0.00;
const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0, at = 0; //Time at which the last beat occurred
String msg = "";
////////////////////////////////////////////////////////////////////////////////////////////
//BITMAPS
//
//ALL bitmap arrays for icons
//
//
////////////////////////////////////////////////////////////////////////////////////////////
// 'drop-15', 24x24px
const unsigned char _drop [] PROGMEM = {
  0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0, 0x0f, 0xf0, 0x1f, 0xf8, 0x1f, 0xf8, 0x3f, 0xfc, 
  0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xfc, 0x3f, 0xf8, 0x1f, 0xf8, 0x0f, 0xe0
};
const unsigned char _cloud [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x1f, 0xf0, 0x3f, 0xf8, 0x3f, 0xf8, 0x3f, 0xf8, 0x7f, 0xfe, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'drops', 24x24px
const unsigned char _drops [] PROGMEM = {
  0x00, 0x00, 0x60, 0x06, 0xf0, 0x0f, 0xf8, 0x1f, 0xf8, 0x1f, 0xf9, 0x9f, 0xf3, 0xcf, 0x07, 0xe0, 
  0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x0f, 0xf0, 0x07, 0xe0, 0x00, 0x00
};
const unsigned char _favorite [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfc, 0x1f, 0xe0, 0x1f, 0xfe, 0x7f, 0xf0, 
  0x3f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfc, 0x7f, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xfe, 
  0x7f, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 
  0x07, 0xff, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0x80, 0x00, 0xff, 0xff, 0x00, 
  0x00, 0x7f, 0xfe, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x0f, 0xf0, 0x00, 
  0x00, 0x07, 0xe0, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'heart', 24x24px
const unsigned char _heart [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x7f, 0xe0, 0x07, 0xfe, 0x00, 0x01, 0xff, 0xf8, 0x1f, 0xff, 0x80, 0x03, 0xff, 
  0xfc, 0x3f, 0xff, 0xc0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 
  0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 
  0xff, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 
  0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 
  0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 
  0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3f, 0xff, 
  0xff, 0xff, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xf8, 
  0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xff, 
  0xff, 0xe0, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0xff, 
  0xff, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xfc, 0x00, 
  0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x07, 0xff, 0xff, 
  0xe0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 
  0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 
  0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 
  0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
// 'heartbeat', 24x24px
const unsigned char _heartbeat [] PROGMEM = {
  0x00, 0x00, 0x00, 0x0f, 0xe7, 0xf0, 0x1f, 0xff, 0xf8, 0x3e, 0xff, 0x7c, 0x78, 0x3c, 0x1e, 0x70, 
  0x18, 0x0e, 0x70, 0x60, 0x0e, 0x60, 0x60, 0x06, 0x60, 0xf0, 0x06, 0x01, 0xf8, 0x00, 0xff, 0xfb, 
  0xff, 0xff, 0x9f, 0xff, 0x04, 0x1f, 0xa0, 0x1c, 0x0f, 0x38, 0x1e, 0x06, 0x78, 0x0f, 0x06, 0xf0, 
  0x07, 0x81, 0xe0, 0x03, 0xc3, 0xc0, 0x01, 0xe7, 0x80, 0x01, 0xff, 0x80, 0x00, 0xff, 0x00, 0x00, 
  0x7e, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x18, 0x00
};
// 'fahrenheit', 24x24px
const unsigned char _fahrenheit [] PROGMEM = {
  0x1f, 0x80, 0x00, 0x3f, 0xc0, 0x00, 0x3f, 0xc0, 0x00, 0x39, 0xc0, 0x00, 0x39, 0xc0, 0x00, 0x3f, 
  0xc0, 0x00, 0x3f, 0xdf, 0xfc, 0x1f, 0x9f, 0xfc, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 
  0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x1f, 0xfc, 0x00, 0x18, 0x00, 
  0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 
  0x18, 0x00, 0x00, 0x18, 0x00, 0x00, 0x18, 0x00
};
const unsigned char _bluetooth [] PROGMEM = {
  0x00, 0x40, 0x00, 0x00, 0x70, 0x00, 0x00, 0x78, 0x00, 0x00, 0x7e, 0x00, 0x18, 0x7f, 0x00, 0x3c, 
  0x67, 0xc0, 0x3e, 0x67, 0xc0, 0x1f, 0x6f, 0x80, 0x07, 0xfe, 0x00, 0x03, 0xfc, 0x00, 0x01, 0xf8, 
  0x00, 0x01, 0xf8, 0x00, 0x03, 0xfc, 0x00, 0x07, 0xfe, 0x00, 0x1f, 0x6f, 0x80, 0x3e, 0x67, 0xc0, 
  0x3c, 0x67, 0xc0, 0x18, 0x6f, 0x80, 0x00, 0x7e, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x78, 0x00, 0x00, 
  0x70, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00
};
// 'position-pin', 24x24px
const unsigned char _pin [] PROGMEM = {
  0x00, 0xff, 0x00, 0x01, 0xff, 0x80, 0x03, 0xff, 0xc0, 0x07, 0x81, 0xe0, 0x0f, 0x00, 0xf0, 0x0e, 
  0x7e, 0x70, 0x0e, 0x7e, 0x70, 0x0e, 0x7e, 0x70, 0x0e, 0x7e, 0x70, 0x0e, 0x7e, 0x70, 0x0e, 0x7e, 
  0x70, 0x0f, 0x00, 0xf0, 0x07, 0x00, 0xe0, 0x07, 0x81, 0xe0, 0x03, 0xc3, 0xc0, 0x01, 0xe7, 0x80, 
  0x0d, 0xe7, 0xb0, 0x7c, 0xff, 0x3e, 0xfe, 0x7e, 0x7f, 0xf0, 0x3c, 0x0f, 0xf0, 0x18, 0x0f, 0xff, 
  0xc3, 0xff, 0x7f, 0xff, 0xfe, 0x0f, 0xff, 0xf8
};
// 'moon', 24x24px
const unsigned char _moon [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x0c, 0x00, 0x00, 0xfc, 0x0c, 0x00, 0x03, 0xfc, 0x3f, 0x00, 
  0x07, 0xfc, 0x3f, 0x00, 0x0f, 0xf8, 0x0c, 0x00, 0x1f, 0xf8, 0x0c, 0x00, 0x1f, 0xfc, 0x00, 0x00, 
  0x3f, 0xfc, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x18, 0x7f, 0xfc, 0x00, 0x18, 0x7f, 0xfe, 0x00, 0x7e, 
  0x7f, 0xfe, 0x00, 0x7e, 0x7f, 0xff, 0x00, 0x18, 0x7f, 0xff, 0x80, 0x18, 0x7f, 0xff, 0xc0, 0x00, 
  0x7f, 0xff, 0xe0, 0x00, 0x7f, 0xff, 0xf8, 0x00, 0x7f, 0xff, 0xff, 0x9e, 0x7f, 0xff, 0xff, 0xfe, 
  0x7f, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xfe, 0x3f, 0xff, 0xff, 0xfe, 0x1f, 0xff, 0xff, 0xfc, 
  0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf8, 0x07, 0xff, 0xff, 0xf0, 0x03, 0xff, 0xff, 0xe0, 
  0x01, 0xff, 0xff, 0xc0, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char _rain [] PROGMEM = {
  0x00, 0x07, 0x03, 0x8f, 0x07, 0x9f, 0x0f, 0xbe, 0x1f, 0x7c, 0x3e, 0xf8, 0x7d, 0xf7, 0xf9, 0xef, 
  0xf1, 0xdf, 0xe0, 0x3e, 0x00, 0x7c, 0x00, 0xf8, 0x01, 0xf0, 0x01, 0xe0, 0x01, 0xc0, 0x00, 0x00
};
// 'sun', 24x24px
const unsigned char _sun [] PROGMEM = {
  0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 
  0x0e, 0x00, 0x00, 0x70, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x80, 0x01, 0xf0, 0x07, 0x0f, 0xf0, 0xe0, 
  0x02, 0x3f, 0xfc, 0x40, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 
  0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0xf1, 0xff, 0xff, 0x8f, 
  0xf1, 0xff, 0xff, 0x8f, 0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0x80, 
  0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x02, 0x3f, 0xfc, 0x40, 
  0x07, 0x0f, 0xf0, 0xe0, 0x0f, 0x80, 0x01, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0e, 0x00, 0x00, 0x70, 
  0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01, 0x80, 0x00
};

// 'temperature', 16x16px
const unsigned char _temperature [] PROGMEM = {
  0x00, 0x0f, 0xc0, 0x00, 0x1f, 0xe0, 0x3f, 0x1f, 0xe0, 0x3f, 0x1f, 0xe0, 0x00, 0x1f, 0xe0, 0x00, 
  0x1f, 0xe0, 0x00, 0x1f, 0xe0, 0x3f, 0x1f, 0xe0, 0x3f, 0x1f, 0xe0, 0x00, 0x1f, 0xe0, 0x00, 0x1f, 
  0xe0, 0x00, 0x3f, 0xf0, 0x3c, 0x7f, 0xf8, 0x3c, 0xff, 0xfc, 0x00, 0xff, 0xfc, 0x00, 0xff, 0xfc, 
  0x00, 0xff, 0xfc, 0x00, 0xe0, 0x3c, 0x00, 0xf0, 0x3c, 0x00, 0xf0, 0x3c, 0x00, 0xfe, 0xfc, 0x00, 
  0x7f, 0xf8, 0x00, 0x3f, 0xf0, 0x00, 0x1f, 0xe0
};


////////////////////////////////////////////////////////////////////////////////////////////
//SETUPRTC
//
//Sets time to compile time, checks if RTC is plugged in, starts and displays to screen
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupRTC(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Real Time Clock."));
  display.display();
  Serial.println("Starting Real Time clock.");
  if (!rtc.begin()) //Use default I2C port, 400kHz speed
  {
    Serial.println("Real Time clock was not found. Please check wiring/power. ");
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Sets time at compile
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPBUTTON
//
//Sets pinmode and button type.
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupButtons(){
  pinMode(MODEBUTTONPIN, INPUT);
  pinMode(SELECTBUTTONPIN, INPUT);
  digitalWrite(MODEBUTTONPIN,HIGH);
  digitalWrite(SELECTBUTTONPIN, HIGH);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPPULSEOXY
//
//
//Starts Heart rate Monitor, displays to screen and sets some default settings (as per libary suggestions)
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupPulseOxy(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Pulse     Oxymeter."));
  delay(250);
  display.display();
  Serial.println("Starting    Pulse Oxymeter");
  pulseOxymeter.begin();
  pulseOxymeter.setup(); //Configure sensor with default settings
  pulseOxymeter.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  pulseOxymeter.setPulseAmplitudeGreen(0); //Turn off Green LED
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPACCELEROMETER
//
//Displays that Accelerometer sensor is starting, checks if sensor is plugged in, calibrates, and starts module
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupAccelerometer(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Accelerometer."));
  display.display();
  Serial.println(F("Starting Accelerometer."));
  mpu.setAddress(0x69); //Sets address since I2C address is shared with RTC I2C address
  mpu.begin();  
  //Serial.println(F("Calculating offsets, do not move MPU6050"));
  //delay(1000);
  //mpu.calcOffsets(true,true); // gyro and accelero
  //Serial.println("Done!\n");
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPIRTEMPSENSOR 
//
//Displays that IR sensor is starting, checks if sensor is plugged in, and starts module
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupIRTempSensor(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  IR Temp   Sensor."));
  display.display();
  Serial.println("Starting IR Temp Sensor");
  if (!mlx.begin())
  {
    Serial.println("IR Temp Sensor was not found. Please check wiring/power. ");
    while (1);
  }
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPDHT
//
//
//Starts Humidity sensor
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupDHT(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Humidity  Sensor."));
  display.display();
  Serial.println("Starting  Humidity Sensor");
  dht.begin();
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPGPS
//
//
//Starts GPS
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupGPS(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  GPS."));
  display.display();
  Serial.println("Starting  GPS");
  gpsPort.begin(9600);
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPDISPLAY
//
//Function sets up the display module, and some default display settings. 
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupDisplay(){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  display.display();
  bigText();
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SETUPBLUETOOTH
//
//Function sets up the display module, and some default display settings. 
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setupBluetooth(){
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Bluetooth."));
  display.display();
  BT.begin(19200); // default baud rate
  delay(DELAY);
}
////////////////////////////////////////////////////////////////////////////////////////////
//BIGTEXT
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void bigText(){
  display.setTextSize(2);
}
////////////////////////////////////////////////////////////////////////////////////////////
//SMALLTEXT
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void smallText(){
  display.setTextSize(1);
}

////////////////////////////////////////////////////////////////////////////////////////////
//SETUP
//
//Main setup function, calls other individual module setup functions and displays when done. 
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(38400);  // start serial for output
  setupDisplay();
  setupDHT();
  setupButtons();
  setupPulseOxy();
  setupIRTempSensor();
  setupAccelerometer();
  setupRTC();
  setupBluetooth();
  setupGPS();
  pinMode(PHOTOSENSORPIN, INPUT);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.print(F("Starting  Loop"));
  display.display();
  delay(250);
  
  for(int i = 0; i<3; i++){
    display.print(F("."));
    display.display();
    buzz();
    delay(250);
  }
  delay(DELAY);
  display.clearDisplay();
  
}

////////////////////////////////////////////////////////////////////////////////////////////
//LOOP
//
//
//Main loop uses switch statement to determine what to display on the screen, navigation using
//the mode button on the device. Checks certain values every loop.
////////////////////////////////////////////////////////////////////////////////////////////
void loop()
{
  
  checkModeButton();
  checkSelectButton();
  readAccel();
  checkLightLevel();
  if(!fallWarning && !lightWarning){
    switch (currentMode) {
      case timeMode:
        homeScreen();
        break;
      case irMode:
        displayIRTemp();
        break;
      case heartMode:
        displayHeartRate();
        break;
      case conMode:
        displayConnection();
        break;
      default:
        break;
    }
  }
  gpsUpdate();
  sendBluetoothData();
}
////////////////////////////////////////////////////////////////////////////////////////////
//DISPLAYCONNECTION
//
//Shows GPS Long and Lat and if connected to Bluetooth
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void displayConnection(){
  smallText();
  display.clearDisplay();
  display.setCursor(7, 0);
  display.println(F("\tConnection Status\t"));
  if(currentSelect == 0){
    display.drawBitmap(10, 18, _bluetooth, 24, 24, WHITE);
    if(BT.available()>0){
      display.setCursor(40, 25);
      display.print(F("Connected"));
    }
    else{
      display.setCursor(35, 25);
      display.print(F("Disconnected"));
    }
    display.setCursor(5, 55);
    display.print(F("\tBluetooth\t    GPS"));
  }
  if(currentSelect == 1){
    display.drawBitmap(10, 18, _pin, 24, 24, WHITE);
    if(gps.available( gpsPort ) || gpsData){
      display.setCursor(45, 10);
      display.print("Lat:");
      display.setCursor(45, 20);
      display.print(lat,6);
      display.setCursor(45, 30);
      display.print("Long:");
      display.setCursor(45, 40);
      display.print(lon,6);
      if(!gpsData){
        gpsData = true;
        gpsDisplayTimer = millis();
      }
      if(millis() - gpsDisplayTimer > 10000){
        gpsData = false;
        gpsDisplayTimer = millis();
      }
    
    }else{
      display.setCursor(45, 25);
      display.print("No GPS Fix");
    }
    display.setCursor(5, 55);
    display.print(F(" Bluetooth    \tGPS\t"));
  }
  display.display();
}
////////////////////////////////////////////////////////////////////////////////////////////
//GPSUPDATE
//
//Updates GPS data
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void gpsUpdate(){
  if(millis() - gpsTimer > 5000){
   if(gps.available( gpsPort )) {
      fix = gps.read();
      if (fix.valid.location) {
        lat = fix.latitude();
        lon = fix.longitude();
        Serial.print("Location:");
        Serial.print(lat);
        Serial.print(",");
        Serial.print(lon);
        Serial.println();
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//BLUETOOTH
//
//Sends out Bluetooth data to a connected device if available
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void sendBluetoothData(){
  if(BT.available()){
    if(millis() - btTimer1 > 10000){
      if(!gpsSend){
        String newMsg  = "";
        msg = newMsg;
        msgNum++;
        if(beatAvg > 50)
          msg = msg +(String)beatAvg +",";
        else
          msg = msg + "N/A"+",";
        if(final_temp > 0)
          msg = msg + (String)final_temp +",";
        else
          msg = msg + "N/A"+",";
        msg = msg+(String)fallWarning+";";
        BT.println(msg);
        Serial.println("Health Data send over bluetooth");
        btTimer1 = millis();
        gpsSend = true;
      }
      else{
        BT.print(lat,6);
        BT.print(",");
        BT.println(lon,6);
        Serial.println("GPS data sent over bluetooth");
        btTimer1 = millis();
        gpsSend = false;
      }
    }
  }
  else{
    if(millis() - btTimer1 > 10000){
      BT.println("asleep");
      btTimer1 = millis();
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//READACCEL
//
//reads the accelerometer data, determines if the user's can has fallen 
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void readAccel(){
  mpu.update();
  at = sqrt(mpu.getAccX()*mpu.getAccX() + mpu.getAccY()*mpu.getAccY() + mpu.getAccZ()*mpu.getAccZ()); //Calulation should average a single digit number between 0 - 4 
  if(at > 2)
    Serial.println(at);
  if(at<1 && sleepFall == false){
    freeFall = true;
    fallIter++;
    fallTimer = millis();
  }
  else if(freeFall && at < 1){
    fallIter++;
  }
  if(fallIter > 100 && at >= 3){ //Checks if after freefall state if there was a crash. 
    fallWarning = true;
    Serial.println(fallIter);
  }
  if(fallWarning){
    if(lightWarning)
      lightWarning = false;
    smallText();
    display.clearDisplay();
    display.setCursor(32, 0);
    display.println(F("\tWarning!\t"));
    display.setCursor(10, 15);
    display.println(F("OBJECT HAS FALLEN"));
    display.setCursor(10, 25);
    display.println(F("Alerting for help"));
    display.setCursor(10, 40);
    display.print(F("Press select to dismiss"));
    display.display();
    if(millis()- buzzTimer > 500){
      buzz();
      buzzTimer = millis();
    }
    if(checkSelectButton()){
      sleepFall = true;
      fallWarning = false;
      fallTimer = millis();
      buzzTimer = 0;
    }
  }
  if(millis() - fallTimer > 5000){
    sleepFall = false;
    fallTimer = millis();
  }  
}
////////////////////////////////////////////////////////////////////////////////////////////
//CHECKLIGHTLEVEL
//
//
//Checks the ambient light level to determine if it may be too dark to be walking right now,
//user can dismiss the warning by pressing select button. 
////////////////////////////////////////////////////////////////////////////////////////////
void checkLightLevel(){
  int lvl = analogRead(PHOTOSENSORPIN);       //Sensor reading inverted, Bright 1 < x < 100 Dim
  if(lvl > 45 && lightWarning == false && sleepLight == false && !fallWarning){
    Serial.println("ITs too dark!");
    lightWarning = true;
  }
  if(lightWarning){
    smallText();
    display.clearDisplay();
    display.setCursor(32, 0);
    display.print(F("\tWARNING!\t"));
    display.setCursor(0, 15);
    display.print(F("It may be too dark towalk"));
    display.setCursor(0, 35);
    display.print(F("Press select to      dismiss"));
    display.display();
    if(millis()- buzzTimer > 500){
      buzz();
      buzzTimer = millis();
    }
    if(checkSelectButton()){
      sleepLight = true;
      lightWarning = false;
      lightTimer = millis();
      buzzTimer = 0;
    }
  }
  if(millis() - lightTimer > 108000){
    sleepLight = false;
    lightTimer = millis();
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//DISPLAYHEARTRATE
//
//Tells user how to check their heart rate using the sensor, displays the users average BPM
//Select button can be used to run again
//
////////////////////////////////////////////////////////////////////////////////////////////
void displayHeartRate(){
  display.clearDisplay();
  display.setCursor(0, 0);
  long irValue = pulseOxymeter.getIR();
  if(finished == true){
    smallText();
    display.clearDisplay();
    display.setCursor(6, 0);
    display.println(F("\tHeart Rate Reader\t"));
    
    display.setCursor(25,10);
    display.print("Average BPM:");
    bigText();
    display.setCursor(30,25);
    display.print(beatAvg);
    display.setCursor(25,43);
    display.print(F("Again?"));
    if(currentSelect == 1){
      finished = false;
      isRunning = false;
      currentSelect = 0;
      pulseOxyTimer = millis();
    }
    display.display();
  }
  else if (irValue < 80000 && isRunning == false){
    smallText();
    display.clearDisplay();
    display.setCursor(6, 0);
    display.println(F("\tHeart Rate Reader\t"));
    display.setCursor(0, 15);
    display.println(F("Place index finger onthe sensor"));
    display.display();
  }
  else if(irValue >90000 && isRunning == false){
    smallText();
    display.clearDisplay();
    display.setCursor(6, 0);
    display.println(F("\tHeart Rate Reader\t"));
    display.setCursor(0, 15);
    display.println(F("Place index finger onthe sensor"));
    display.setCursor(0, 40);
    display.print(F("Press select to begin"));
    display.display();
    if(checkSelectButton()){
      pulseOxyTimer = millis();
      currentSelect = 0;
      display.clearDisplay();
      display.setCursor(6, 0);
      display.println(F("\tHeart Rate Reader\t"));
      display.setCursor(0, 15);
      display.print(F("Keep holding finger down"));
      Serial.println("reading...");
      display.display();
      delay(1000);
    }
  }
  else if (irValue < 80000 && isRunning == true){
    isRunning = false;
  }
  while (checkForBeat(irValue) == true && isRunning == true && finished == false)
  {
    smallText();
    display.clearDisplay();
    display.setCursor(6, 0);
    display.println(F("\tHeart Rate Reader\t"));
    
    if(!heartFull){
      display.drawBitmap(46, 16, _favorite, 32, 32, WHITE);
      heartFull = true;
    }
    else{
      display.drawBitmap(40, 10, _heart, 48, 48, WHITE);
      heartFull = false;
    }
    delay(100);
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20){
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
      
    }
    if(millis() - pulseOxyTimer > 30000){
      isRunning = false;
      finished = true;
      pulseOxyTimer = millis();
    }
    display.display();
    Serial.print(F(", BPM:"));
    Serial.print(beatsPerMinute);
    Serial.print(", BPM: ");
    Serial.println(beatAvg);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//BUZZ
//
//
//Plays a tone on the buzzer 
//
////////////////////////////////////////////////////////////////////////////////////////////
void buzz() {
  tone(BUZZERPIN, PITCH);
  delay(250);
  noTone(BUZZERPIN);
  delay(250);
}
////////////////////////////////////////////////////////////////////////////////////////////
//DISPLAYIRTEMP
//
//Prompts user to press select button to read targets temperature, reads temperature and displays 
//the target temperature on the screen
//
////////////////////////////////////////////////////////////////////////////////////////////
void displayIRTemp() {
  display.clearDisplay();
  if(isRunning == false && finished == false){
    smallText();
    display.clearDisplay();
    display.setCursor(13, 0);
    display.print(F("\tIR Temp Reader\t"));
    display.setCursor(0, 15);
    display.println(F("Point cane handle at target to read."));
    display.setCursor(0, 40);
    display.println(F("Press select to begin"));
    irTimer = millis();
    display.display();
  }
  if(isRunning == true && finished == false){
    smallText();
    display.clearDisplay();
    display.setCursor(13, 0);
    display.print(F("\tIR Temp Reader\t"));
    bigText();
    display.setCursor(25, 23);
    display.print("Reading");
    display.setCursor(25, 40);
    display.print("Temp.");
    avg_temp = avg_temp + mlx.readObjectTempF();
    iter++;
    if (millis() - irTimer > 7000) {
      avg_temp = avg_temp / iter;
      final_temp = avg_temp;
      iter = 0;
      avg_temp = 0;
      irTimer = millis();
      isRunning = false;
      finished = true;
      currentSelect = 0;
    }
    display.display();
  }
  if(finished == true){
    display.clearDisplay();
    smallText();
    display.clearDisplay();
    display.setCursor(13, 0);
    display.print(F("\tIR Temp Reader\t"));
    display.setCursor(25, 10);
    display.print(F("Temperature:"));
    bigText();
    display.setCursor(30, 23);
    display.print(final_temp, 1); display.print("F");
    display.setCursor(25, 45);
    display.print("Again?");
    display.display();
    if(checkSelectButton() == true){
      finished = false;
      currentSelect = 0;
    }
    isRunning = false;
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//DISPLAYTIME
//
//Main screen, displays current date and time in 24hr format on the screen and updates every 
//second. Also shows what day of the week and Month user is in. 
//
////////////////////////////////////////////////////////////////////////////////////////////
void homeScreen() {
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  DateTime now = rtc.now();
  
  //DAY OF WEEK
  bigText();
  display.clearDisplay();
  display.setCursor(0, 11);
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  

  //Conditional for Weather Icon (Based on humidity and Time)
  int humidityValue = event.relative_humidity;
  display.setCursor(98,0);
  display.print(humidityValue);
  smallText();
  display.print(F("%"));
  if(humidityValue >= 25 && humidityValue < 35)
      display.drawBitmap(100, 18, _drop, 16, 16, WHITE);
  else if(humidityValue >= 35 && humidityValue < 55)
      display.drawBitmap(100, 18, _drops, 16, 16, WHITE);
  else if(humidityValue < 25)
      display.drawBitmap(100, 18, _cloud, 16, 16, WHITE);
  else if(humidityValue >= 55)
      display.drawBitmap(100, 18, _rain, 16, 16, WHITE);
  
  
  //DATE
  display.setCursor(0,0);
  display.print(now.month());
  display.print('/');
  display.print(now.day());
  display.print('/');
  display.print(now.year());
  
  //Temperature Stats
  display.drawBitmap(62, 38, _temperature, 24, 24, WHITE);
  display.setTextSize(3);
  display.setCursor(90,40);
  float C = event.temperature;
  int F = (C * (9/5))+32;
  display.print(F);

  //TIME
  display.setCursor(0,30);                            
  bigText();
  if(now.hour() > 12){
    if(now.hour()-12 < 10)
      display.print(F("0"));
    display.print(now.hour()-12);
    display.print(F(":"));
    if(now.minute() < 10)
      display.print(F("0"));
    display.print(now.minute());
  }
  else{
    if(now.hour() < 10)
      display.print(F("0"));
    display.print(now.hour());
    display.print(F(":"));
    if(now.minute() < 10)
      display.print(F("0"));
    display.print(now.minute());
  }
  if(now.hour() > 6 && now.hour() < 19){
    display.drawBitmap(58, 0, _sun, 32, 32, WHITE);
    night = false;
  }
  else{
    night = true;
    display.drawBitmap(58, 0, _moon, 32, 32, WHITE);
  }
  //Seconds
  display.setCursor(0,50);
  display.print(now.second());
  if(now.second() < 10)
    display.print(F(" "));
  if(night)
     display.print(F(" PM"));
  else
     display.print(F(" AM"));
  display.display();
}
////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////
void checkModeButton() {
  buttonState = digitalRead(MODEBUTTONPIN);
  //delay(buttonDelay);
  if (buttonState == LOW) {
    while(buttonState != HIGH){
      delay(10);
      buttonState = digitalRead(MODEBUTTONPIN);
    }
    if (buttonState == HIGH) {
      Serial.println("Mode Button Pressed");
      currentMode += 1;
      isRunning = false;
      finished = false;
      currentSelect = 0;
      if (currentMode > numModes - 1) {
        currentMode = 0;
      }
    }
  }
}
////////////////////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////
boolean  checkSelectButton() {
  buttonState = digitalRead(SELECTBUTTONPIN);
  //delay(buttonDelay);
  if (buttonState == LOW) {
    //digitalWrite(LASERPIN,HIGH);
    while(buttonState != HIGH){
      delay(10);
      buttonState = digitalRead(SELECTBUTTONPIN);
    }
    if (buttonState == HIGH) {
      Serial.println("Select Button Pressed");
      currentSelect += 1;
      if (currentSelect > 1) {
        currentSelect = 0;
      }
      if(currentMode == irMode || currentMode == heartMode || currentMode == conMode)
        isRunning = true;
      return true;
    }
  }
  return false;
}
