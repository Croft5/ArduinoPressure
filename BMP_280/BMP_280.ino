/***************************************************************************
  This is a library for the BMP3XX temperature & pressure sensor

  The sensors use I2C.
***************************************************************************/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>

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
unsigned long historicTime;

Adafruit_BME280 bme; // I2C

void setup() 
{
    Serial.begin(9600);
    while (!Serial);
    Serial.println(F("BME280 test"));
  
    if (!bme.begin()) 
    {   // hardware I2C mode, can pass in address & alt Wire
      Serial.println(F("Could not find a valid BMP3 sensor, check wiring!"));
      while (1);
    }
  
    // initialize the LCD
    lcd.begin();

    // Turn on the blacklight
    lcd.backlight();

    analogWrite(BRIGHTNESS_PIN, 100);

    // Write the parts that don't change
    lcd.setCursor(0, 0);
    lcd.print(F("Pressure "));
 
    lcd.setCursor(0, 1);
    lcd.print(F("Rate "));

    historicPressure1 = historicPressure2 = (bme.readPressure()/ 100.0F) + ADJUSTMENT;
    historicTime = 0;   
}

void loop() 
{
    unsigned long now = millis();
    printPressure(now);
    delay(delayTime);
}


int minuteOfLastUpdate = -1;

void printPressure(unsigned long now) 
{
    // Only update if the minute has changed
    unsigned long minuteNow = now / 60000;
    if(minuteNow == minuteOfLastUpdate)
    {
        return;
    }
    minuteOfLastUpdate = minuteNow;
        
    // Calculate pressure
    float adjustedValue = (bme.readPressure()/ 100.0F) + ADJUSTMENT;

    // Calculate historic pressure
    unsigned long nextUpdate = historicTime + 3600000;
    if(nextUpdate <= now)
    {
         historicTime = nextUpdate;
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

    
}

void printPaddedNumber(int number)
{
    if(number < 10)
    {
        lcd.print(F("0"));
    }
    lcd.print(number);
}
