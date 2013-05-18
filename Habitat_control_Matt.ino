/*
Sketch to automate 2 separate enviroments including: 
Hot and cold end temperature sensors
Four configurable 240V power switches that can be time or probe controlled
With enviroments sharing a RGB LED light cycle and ventilation/cooling via 12VDC fans

Sketch can be altered to automate upto 4 enviroments by reducing to only hotend temperature sensors

Only adjust those settings in the User Adjustable section unless confident with altering code!

Wishlist:  (Dependant on priority and job list size these will filter down for development)
1.
2.
3.
4.

Development list:  (Priority)
1. Add accurate clock/timing system for lighting
2. Increase code to two enviroments
3. LCD scrolling for above
4. Increase user definable options

Jobs list:  (Implemented in code though untested)
1. LCD size variables on cooling & heating
2. 1 setup and 2 setups
3. 
4. 
*/

#include <Wire.h>
#include "RTClib.h"
#include "constants.h"                // Define constants
#include <LiquidCrystal.h>            // LCD display library

//*************************************************************************************************************
//**************************************** USER ADJUSTABLE ****************************************************
//*************************************************************************************************************

//===================================== Temperature settings ==================================================

int fahrenheit(true);                 // Set to false for Celsius
int fans(false);                       // Set to false if not using fans

// Enviroment 1 (V1)
//int hotend1(88);                      // Adjust for hotend temperature
int coolend(90);                      // Adjust for coolend temperature

// Enviroment 2 (V2)
int hotend2(72);                      // Adjust for hotend temperature
//int coolend2();                       // Adjust for coolend temperature

//========================================= Time settings =====================================================

//int LED(true);                        // Set to false if LEDs are not being used
//int full_cycle(true);                 // Set to false if LEDs are only for moon lighting

//int moonrise();                       // Nil    ~   Blue
//int sunrise();                        // Blue   ~   White
//int sunset();                         // White  ~   Blue
//int moonset();                        // Blue   ~   Nil

//======================================== Display options ====================================================

int LCD(true);                        // Set to false if no LCD display is used to allow serial communication
#define LCD_WIDTH 20                  // 16 or 20 dependant on the amount of characters your LCD has
#define LCD_HEIGHT 4                  // 2 or 4 dependant on the amount of lines your LCD has

//*************************************************************************************************************
//*********************************************** END *********************************************************
//*************************************************************************************************************

int samples[numsamples];              // Amount of temperature samples to take

LiquidCrystal lcd(0, 1, 2, 4, 7, 13); // LCD interface pins
RTC_DS1307 RTC;

// Create characters (can have up to 8)
byte degree[8] = {
  B01100,
  B10010,
  B10010,
  B01100,
  B00000,
  B00000,
  B00000,
};

byte thermometer[8] = {
  B00100,
  B01010,
  B01010,
  B01010,
  B10001,
  B10001,
  B01110,
  B00000,
};

byte poweroff[8] = {
  B01110,
  B10001,
  B11011,
  B10101,
  B11011,
  B10001,
  B11111,
};

byte power25[8] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B10011,
  B11111,
  B11111,
};

byte power50[8] = {
  B01110,
  B10001,
  B10001,
  B10111,
  B11111,
  B11111,
  B11111,
};

byte power100[8] = {
  B01110,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
};

byte sun[8] = {
  B10101,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B10101,
};

byte moon[8] = {
  B00111,
  B01010,
  B10100,
  B10100,
  B10100,
  B01010,
  B00111,
};

void setup()  {
  digitalWrite(relay_1, relay_OFF);  // Initialize Pins so relays are inactive at reset
  digitalWrite(relay_2, relay_OFF);
  digitalWrite(relay_3, relay_OFF);
  digitalWrite(relay_4, relay_OFF);
  
  pinMode(relay_1, OUTPUT);          // Then set pins as OUTPUT
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  
  analogReference(EXTERNAL);         // To improve temperature accuracy
  
  delay(1000);                       // Delay to allow LCD to initilize
  
// Create custom characters
  lcd.createChar(0, degree);         // Degree symbol for temperature
  lcd.createChar(1, thermometer);    // Thermometer symbol for temperature
  lcd.createChar(2, poweroff);       // Power OFF symbol
  lcd.createChar(3, power25);        // Power at 25% symbol
  lcd.createChar(4, power50);        // Power at 50% symbol
  lcd.createChar(5, power100);       // Power at 100% symbol
  lcd.createChar(6, sun);            // Sun symbol for daytime
  lcd.createChar(7, moon);           // Moon symbol for nighttime

// Output of data setup
  if(LCD==true)  {  
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);  // Setup of LCD columns and rows
  lcd.clear();
  }
  else  {
  Serial.begin(9600);                // Setup serial link for debugging
  Serial.println("Working");         // Prove communication
  }
  
  //Serial.begin(57600);
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  digitalWrite(relay_2, relay_ON);   // Lights on
  lcd.setCursor(11, 1);
  lcd.print("Light ON");
  delay(300000);                     // 5 minute wait before continuing the sketch
  digitalWrite(relay_2, relay_OFF);
  lcd.setCursor(11, 1);
  lcd.print("Light OFF");            // Lights off
}

void loop()  {  
  heat1();                           // Check hotend temperature
  cool1();                           // Check coolend temperature
  
  DateTime now = RTC.now();          // Records the time from the RTC
  lcd.setCursor(0, 0);               // Move cursor
  lcd.print("                    "); // Clear the line
  if(now.day()<10)  {
    lcd.setCursor(0, 0);
    lcd.print('0');
    lcd.setCursor(1, 0);
    lcd.print(now.day(), DEC);
  }
  else  {
    lcd.setCursor(0, 0);
    lcd.print(now.day(), DEC);
  }
  lcd.setCursor(2, 0);
  lcd.print('/');
  if(now.month()<10)  {
    lcd.setCursor(3, 0);
    lcd.print('0');
    lcd.setCursor(4, 0);
    lcd.print(now.month(), DEC);
  }
  else  {
    lcd.setCursor(3, 0);
    lcd.print(now.month(), DEC);
  }
  lcd.setCursor(5, 0);
  lcd.print('/');
  lcd.setCursor(6, 0);
  lcd.print(now.year(), DEC);
  if(now.hour()<10)  {
    lcd.setCursor(11, 0);
    lcd.print('0');
    lcd.setCursor(12, 0);
    lcd.print(now.hour(), DEC);
  }
  else  {
    lcd.setCursor(11, 0);
    lcd.print(now.hour(), DEC);
  }
  lcd.setCursor(13, 0);
  lcd.print(':');
  if(now.minute()<10)  {
    lcd.setCursor(14, 0);
    lcd.print('0');
    lcd.setCursor(15, 0);
    lcd.print(now.minute(), DEC);  
  }
  else  {
    lcd.setCursor(14, 0);
    lcd.print(now.minute(), DEC);
  }
  
  if(now.hour()<7)  {
    heat2();                           
  }  
  else if(now.hour()<10)  {
    digitalWrite(relay_1, relay_OFF);
    lcd.setCursor(0, 3);
    lcd.print("                    ");
    digitalWrite(relay_2, relay_ON);
    lcd.setCursor(11, 1);
    lcd.print("Light ON");
    digitalWrite(relay_3, relay_ON);
    lcd.setCursor(11, 2);
    lcd.print("Lamp ON"); 
  }
  else if(now.hour()<13) {
    digitalWrite(relay_2, relay_OFF);
    lcd.setCursor(11, 1);
    lcd.print("Light OFF");    
    digitalWrite(relay_3, relay_OFF);
    lcd.setCursor(11, 2);
    lcd.print("Lamp OFF");    
  }
  else if(now.hour()<22) {
    digitalWrite(relay_2, relay_ON);
    lcd.setCursor(11, 1);
    lcd.print("Light ON");    
    digitalWrite(relay_3, relay_ON);
    lcd.setCursor(11, 2);
    lcd.print("Lamp ON");    
  }
  else if(now.hour()<23)  {
    digitalWrite(relay_2, relay_OFF);
    lcd.setCursor(11, 1);
    lcd.print("Light OFF");    
    digitalWrite(relay_3, relay_OFF);
    lcd.setCursor(11, 2);
    lcd.print("Lamp OFF");    
    evening();
    heat2();
  }
  else  {
    dark();
    heat2();
  }
  
  delay(4000);
}
