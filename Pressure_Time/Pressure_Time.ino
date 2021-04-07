/***************************************************************************
  This is a library for the BMP3XX temperature & pressure sensor

  Designed specifically to work with the Adafruit BMP388 Breakout
  ----> http://www.adafruit.com/products/3966

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP3XX.h"
#include <LiquidCrystal_I2C.h>
#include "RTClib.h"

#define SEALEVELPRESSURE_HPA (1013.25)

// This is used to adjust reading to sealevel
#define ADJUSTMENT (8.30f)

// This adjusts the backlight brightness
#define BRIGHTNESS_PIN 6

// Set the LCD address to 0x27 for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 20, 4);

//Vairables
unsigned long delayTime;
float historicPressure1 = 0.0f;
float historicPressure2 = 0.0f;
DateTime historicTime;

Adafruit_BMP3XX bmp;

RTC_DS3231 rtc;

char daysOfTheWeek[7][5] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char monthsOfTheYear[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void setup() 
{
    Serial.begin(9600);
    while (!Serial);
    Serial.println(F("Adafruit BMP388 / BMP390 test"));
  
    if (!bmp.begin_I2C()) 
    {   // hardware I2C mode, can pass in address & alt Wire
      Serial.println(F("Could not find a valid BMP3 sensor, check wiring!"));
      while (1);
    }
  
    // Set up oversampling and filter initialization
    bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  
    if (! bmp.performReading()) 
    {
        Serial.println(F("Failed to perform reading :("));
        return;
    }

    // initialize the LCD
    lcd.begin(20, 4);

    // Turn on the blacklight
    lcd.backlight();

    analogWrite(BRIGHTNESS_PIN, 80);

    // This section allows you to input a future time and pasue the program until reached
    
    //configure pin 12 as an input and enable the internal pull-up resistor
    pinMode(12, INPUT_PULLUP);
    int val = 0;

     int inPin = 12;
     val = digitalRead(inPin); // stores the value of pin 12 as High or Low

        while (val== LOW){
        DateTime now = rtc.now();
        rtc.adjust(DateTime(2021, 04, 07, 12, 28, 00)); // Year, Month, Date, Hour, Min, Sec
        lcd.setCursor(0, 1);
        lcd.print(F("Release button at"));
        printTime(now);
        val = digitalRead(inPin);
      }
     
    
    // Write the parts that don't change
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Pressure "));
 
    lcd.setCursor(0, 1);
    lcd.print(F("Rate "));

    historicPressure1 = historicPressure2 = (bmp.readPressure()/ 100.0F) + ADJUSTMENT;
    DateTime now = rtc.now();
    historicTime = DateTime(now.year(), now.month(), now.day(), now.hour(), 0, 0);   
}

void loop() 
{
    DateTime now = rtc.now();
    printTime(now);
    printPressure(now);
    delay(delayTime);
}

void printTime(DateTime now)
{
    // Display Time
    lcd.setCursor(0, 3);

    // Day & Month
    lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    lcd.print(F(" "));
    printPaddedNumber(now.day());
    lcd.print(F(" "));
    lcd.print(monthsOfTheYear[now.month() - 1]);

    // Hours, minutes & Seconds
    lcd.print(F(" "));
    printPaddedNumber(now.hour());
    lcd.print(F(":"));
    printPaddedNumber(now.minute());
    lcd.print(F(":"));
    printPaddedNumber(now.second());
}

int minuteOfLastUpdate = -1;

void printPressure(DateTime now) 
{
    // Only update if the minute has changed
    if(now.minute() == minuteOfLastUpdate)
    {
        return;
    }
    minuteOfLastUpdate = now.minute();
        
    // Calculate pressure
    float adjustedValue = (bmp.readPressure()/ 100.0F) + ADJUSTMENT;

    // Calculate historic pressure
    DateTime nextUpdate = historicTime + TimeSpan(0, 1, 0, 0);
    if(nextUpdate <= now)
    {
         historicTime = DateTime(now.year(), now.month(), now.day(), now.hour(), 0, 0);
         historicPressure2 = historicPressure1;
         historicPressure1 = adjustedValue;
    }
    float pressureChange = historicPressure1 - historicPressure2;

    // Display pressure
    //Serial.print("Pressure = ");
    //Serial.print(adjustedValue);
    //Serial.println(" hPa");

    lcd.setCursor(9, 0);
    lcd.print(adjustedValue);
    lcd.print(F(" mb "));

    // Display Pressure Change
    lcd.setCursor(5, 1);
    lcd.print(pressureChange);
    lcd.print(F(" mb/h "));

    // Display Pressure Change Status
    lcd.setCursor(0, 2);
    if(pressureChange >= 0.1f)
    {
        if(pressureChange >= 0.5f)
        {
            if(pressureChange >= 1.0f)
            {
                lcd.print(F("Warning        "));
            }
            else
            {
                lcd.print(F("Rising Rapidly "));
            }
        }
        else
        {
            lcd.print(F("Rising         "));
        }
    }
    else if(pressureChange <= -0.1f)
    {
        if(pressureChange <= -0.5f)
        {
            if(pressureChange <= -1.0f)
            {
                lcd.print(F("Warning        "));
            }
            else
            {
                lcd.print(F("Falling Rapidly"));
            }
        }
        else
        {
            lcd.print(F("Falling        "));
        }
    }
    else
    {
        lcd.print(F("Steady         "));
    }

    //Serial.print("Approx. Altitude = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //Serial.println(" m");

    //Serial.print("Humidity = ");
    //Serial.print(bme.readHumidity());
    //Serial.println(" %");

    //Serial.println();
}

void printPaddedNumber(int number)
{
    if(number < 10)
    {
        lcd.print(F("0"));
    }
    lcd.print(number);
}
