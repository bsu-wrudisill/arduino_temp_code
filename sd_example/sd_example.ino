/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library. Pin numbers reflect the default
  SPI pins for Uno and Nano models

  The circuit:
   analog sensors on analog pins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** SDO - pin 11
 ** SDI - pin 12
 ** CLK - pin 13
 ** CS - depends on your SD card shield or module.
 		Pin 10 used here for consistency with other Arduino examples
    (for MKR Zero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified  24 July 2020
  by Tom Igoe

  This example code is in the public domain.

FROM AMAZON: SD CARD ADAPTER
# https://www.amazon.com/dp/B07X478BPL?ref=ppx_yo2ov_dt_b_fed_asin_title
Works perfectly with Arduino Uno. Be sure to select CS, chip select = pin 4.
Built-in sd demo in the Arduino IDE. SCK pin 13, MOSI pin 11, MISO pin 12
Vcc to 5v and gnd to gnd


// FOR THE RTC CLOCK
// https://www.instructables.com/How-to-Create-a-Clock-Using-Arduino-DS3231-RTC-Mod/
// SDA -- A4
// SCL -- A5 

*/

// INFO ABOUT LOW POWER DESIGN
// https://www.gammon.com.au/power



// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

#include <NewPing.h>

#include <avr/sleep.h>


#define TRIGGER_PIN  6  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     8  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 400 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.




// constants and declarations up here...
RTC_DS3231 rtc;
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
const int chipSelect = 4;

// THIS IS FOR THE THERMISTOR
#define SERIESRESISTOR 10000    
// What pin to connect the sensor to
#define THERMISTORPIN A0 
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
int samples[NUMSAMPLES];


// Do the setup
void setup () {
  Serial.begin(9600);

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
     }

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
  sleep_cpu ();  

}

void loop () { 

      // ###########    ###########    ###########  
    // NOW DO THE .... TEMPERATURE READING 
    // ###########    ###########    ###########
    // https://learn.adafruit.com/thermistor/using-a-thermistor
    uint8_t i;
    float average;
    float reading;

    // take N samples in a row, with a slight delay
    for (i=0; i< NUMSAMPLES; i++) {
    samples[i] = analogRead(THERMISTORPIN);
    delay(10);
    }
    
    // average all the samples out
    average = 0;
    for (i=0; i< NUMSAMPLES; i++) {
      average += samples[i];
    }
    average /= NUMSAMPLES;

    // Serial.print("Average analog reading "); 
    // Serial.println(average);
    
    // convert the value to resistance
    average = 1023 / average - 1;
    average = SERIESRESISTOR / average;
    // Serial.print("Thermistor resistance "); 
    // Serial.println(average);
    
    float steinhart;
    steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
    steinhart = log(steinhart);                  // ln(R/Ro)
    steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    steinhart = 1.0 / steinhart;                 // Invert
    steinhart -= 273.15;                         // convert absolute temp to C
    
    // Serial.print(steinhart);
    // ###########    ###########    ###########  
    // END TEMPERATURE READING 
    // ###########    ###########    ###########
  
   // ###########    ###########    ###########  
   // NEXT
   // ###########    ###########    ###########  

   DateTime now = rtc.now();

   char dataString[50];
   char dataString2[10]; 
   char dataString3[10]; 
   char dataString4[10]; 

   char comma[] = ",";

   float temperature = rtc.getTemperature();

    // Format the datetime and append 'foo'
    sprintf(dataString, "%04d-%02d-%02d %02d:%02d:%02d",
          now.year(), now.month(), now.day(),
          now.hour(), now.minute(), now.second()
          );

    dtostrf(temperature, 4, 1, dataString2);  // (value, minWidth, decimalPlaces, buffer)
    dtostrf(sonar.ping_cm(), 4, 1, dataString3);  // (value, minWidth, decimalPlaces, buffer)
    dtostrf(steinhart, 4, 1, dataString4);  // (value, minWidth, decimalPlaces, buffer)

    strcat(dataString, comma);
    strcat(dataString, dataString2);  
    strcat(dataString, comma);
    strcat(dataString, dataString3);  
    strcat(dataString, comma);
    strcat(dataString, dataString4);  

    File dataFile = SD.open("datalog.txt", FILE_WRITE);

    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println(dataString); // For debugging
      } else {
        Serial.println("Error opening file");
      }



    delay(5000);
}