// Useful links to things.
// steinhart calculator https://thinksrs.com/downloads/programs/therm%20calc/ntccalibrator/ntccalculator.html
// bad(?) instructions for thermistors -- i have not wired them this way https://learn.adafruit.com/thermistor/using-a-thermistor
// another one https://www.build-electronic-circuits.com/arduino-thermistor/
// info about the multiplexer http://adam-meyer.com/arduino/CD74HC4067
// seemingly good info about the arduino ADC     // https://skillbank.co.uk/arduino/measure2.htm



#include "RTClib.h"
double a = 1.114592028E-3;
double b = 2.366194439E-4;
double c = 0.7691014540E-7;



int SensorValue; 
float R1 = 9800;
float Vin = 4.6; 

float R2; 
float V1;
float V2; 
float logR2;
float T; 



//Mux control pins
int s0 = 5;
int s1 = 4;
int s2 = 3;
int s3 = 2;

//Mux in "SIG" pin
int SIG_pin = 1;


RTC_DS3231 rtc;

void setup(){
  pinMode(s0, OUTPUT); 
  pinMode(s1, OUTPUT); 
  pinMode(s2, OUTPUT); 
  pinMode(s3, OUTPUT); 

  digitalWrite(s0, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);

  Serial.begin(9600);

  // if (! rtc.begin()) {
  //   Serial.println("Couldn't find RTC");
  //   Serial.flush();
  //   while (1) delay(10);
  // }


}




void loop(){

  //Loop through and read all 16 values
  //Reports back Value at channel 6 is: 346
  for(int i = 11; i < 12; i ++){
    // read the sensor...
    SensorValue = readMux(i); 
    // V2 is the voltage of the resistor 
    // https://skillbank.co.uk/arduino/measure2.htm
    V2= (SensorValue + 0.5) * (Vin / 1024.0);
    
    // now convert the voltage to... resistance 
    // compute the current I0 which is constant across the circuit...
    // we can do this because we know the resistance of the known
    // resistor 

    // this is the voltage across the resistor 
    V1 = Vin - V2;

    // I0 = V1/R1 
    R2 = (V1/V2) * R1 ; 

    // 
    logR2 = log(R2);
    T = 1.0 / (a + b*logR2 + c*logR2*logR2*logR2); 
    T = T - 273.15;

    Serial.print(V1);
    Serial.print(",");
    Serial.print(R2);
    Serial.print(",");
    Serial.println(T);

  

    delay(1000);

  }

    // DateTime now = rtc.now(); // get the datetime from the clock 
    // char dataString[50];
    // sprintf(dataString, "%04d-%02d-%02d %02d:%02d:%02d",
    //       now.year(), now.month(), now.day(),
    //       now.hour(), now.minute(), now.second()
    //       );


  // Serial.println(dataString);


}


int readMux(int channel){
  int controlPin[] = {s0, s1, s2, s3};

  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  //loop through the 4 sig
  for(int i = 0; i < 4; i ++){
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  //read the value at the SIG pin
  int val = analogRead(SIG_pin);

  //return the value
  return val;
}