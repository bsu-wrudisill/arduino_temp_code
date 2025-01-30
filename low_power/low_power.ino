#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <SPI.h>
#include <SD.h>

// for the SD card 
const int chipSelect = 2;

// to power the SD card 
#define CLOCKPOWER 8
#define SDPOWER 4



RTC_DS3231 rtc;
bool century = false;
bool h12Flag;
bool pmFlag;


const byte LED = 9;


void flash ()
  {
  pinMode (LED, OUTPUT);
  for (byte i = 0; i < 10; i++)
    {
    digitalWrite (LED, HIGH);
    delay (50);
    digitalWrite (LED, LOW);
    delay (50);
    }
    
  pinMode (LED, INPUT);
    
  }  // end of flash



// watchdog interrupt
ISR (WDT_vect) 
{
   wdt_disable();  // disable watchdog
}  // end of WDT_vect
 


void setup () {  // end of setup
  pinMode(9, OUTPUT);
  pinMode(CLOCKPOWER, OUTPUT);
  digitalWrite (CLOCKPOWER, HIGH);

  pinMode(SDPOWER, OUTPUT);
  digitalWrite (SDPOWER, HIGH);
  delay(500); // sleep for just a half second 


  Serial.begin(57600);
  // #ifndef ESP8266
  // while (!Serial); // wait for serial port to connect. Needed for native USB
  // #endif

  // DO THE ... RTC CLOCK THING
  rtc.begin();
  // if (! rtc.begin()) {
  //   Serial.println("Couldn't find RTC");
  //   Serial.flush();
  //   while (1) delay(10);
  // }
  // else{ Serial.print("HERE");}
  // if (rtc.lostPower()) {
  //   Serial.println("RTC lost power, let's set the time!");
  //   // When time needs to be set on a new device, or after a power loss, the
  //   // following line sets the RTC to the date & time this sketch was compiled
  //   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //   // This line sets the RTC with an explicit date & time, for example to set
  //   // January 21, 2014 at 3am you would call:
  //   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  // }


    /// START UP THE SD CARD 
    if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
     }



    // make the clock and the sd card go to sleep
    digitalWrite (CLOCKPOWER, LOW);
    digitalWrite (SDPOWER, LOW);
    delay(500); // sleep for just a half second 


}

// the loop function runs over and over again forever
void loop() {
  flash ();
  // pinMode (CLOCKPOWER, OUTPUT);
  digitalWrite (CLOCKPOWER, HIGH);
  digitalWrite (SDPOWER, HIGH);



  DateTime now = rtc.now();

  char dataString[50];

  char comma[] = ",";

  float temperature = rtc.getTemperature();

  // Format the datetime and append 'foo'
  sprintf(dataString, "%04d-%02d-%02d %02d:%02d:%02d",
        now.year(), now.month(), now.day(),
        now.hour(), now.minute(), now.second()
        );
   
  Serial.println(dataString); // For debugging
  
  // WRITE STUFF TO A FILE 
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString); // For debugging
    } else {
      Serial.println("Error opening file");
    }

  // finished with clock
  // pinMode (CLOCKPOWER, INPUT);  
  digitalWrite (CLOCKPOWER, LOW);
  digitalWrite (SDPOWER, LOW);
 



  ////////////////////////////////
  // DO THE SLEEPING BELOW HERE //
  ////////////////////////////////

  // disable ADC
  ADCSRA = 0;  

  // clear various "reset" flags
  MCUSR = 0;     
  // allow changes, disable reset
  WDTCSR = bit (WDCE) | bit (WDE);
  // set interrupt mode and an interval 
  WDTCSR = bit (WDIE) | bit (WDP3) | bit (WDP0);    // set WDIE, and 8 seconds delay
  wdt_reset();  // pat the dog
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  noInterrupts ();           // timed sequence follows
  sleep_enable();
 
  // turn off brown-out enable in software
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS); 
  interrupts ();             // guarantees next instruction executed
  sleep_cpu ();  
  
  // cancel sleep as a precaution
  sleep_disable();
  
}

