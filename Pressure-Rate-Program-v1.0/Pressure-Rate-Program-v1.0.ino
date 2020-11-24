/* Pressure and Rate program by David & Stephen Edmonds  
 ***************************************************************************/
//These are the libarys required
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

//This is the interface for the sensor
Adafruit_BME280 bme; // I2C

//Vairables
unsigned long delayTime;
float historicPressure1 = 0.0f;
float historicPressure2 = 0.0f;
unsigned long historicTime = 0;

// Boot info
void setup() {
    Serial.begin(9600);
    while(!Serial);    // time to get serial running
    Serial.println(F("BME280 test"));
    
    // initialize the LCD
    lcd.begin();

    // Turn on the blacklight
    lcd.backlight();

    // Write the parts that don't change
    lcd.setCursor(0, 0);
    lcd.print("Pressure ");
    
    lcd.setCursor(0, 2);
    lcd.print("Rate ");

    analogWrite(BRIGHTNESS_PIN, 100);
    
    unsigned status;
    
    // default settings
    status = bme.begin(); 
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
        Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
        Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
        Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
        Serial.print("        ID of 0x60 represents a BME 280.\n");
        Serial.print("        ID of 0x61 represents a BME 680.\n");
        while (1) delay(10);
    }

    historicPressure1 = historicPressure2 = (bme.readPressure()/ 100.0F) + ADJUSTMENT;
    
    Serial.println("-- Default Test --");
    delayTime = 60000;

    Serial.println();
}


void loop() { 
    printValues();
    delay(delayTime);
}


void printValues() {
    
    // Calculate pressure
    float adjustedValue = (bme.readPressure()/ 100.0F) + ADJUSTMENT;

    // Calculate historic pressure
    unsigned long timeNow = millis();
    unsigned long nextUpdate = historicTime + (60UL * 60UL * 1000UL);
    if(nextUpdate < timeNow)
    {
         historicTime = timeNow;
         historicPressure2 = historicPressure1;
         historicPressure1 = adjustedValue;
    }
    float pressureChange = historicPressure1 - historicPressure2;

    // Display pressure
    Serial.print("Pressure = ");
    Serial.print(adjustedValue);
    Serial.println(" hPa");

    lcd.setCursor(9, 0);
    lcd.print(adjustedValue);
    lcd.print(" mb ");

    // Display historic pressure
    lcd.setCursor(5, 2);
    lcd.print(pressureChange);
    lcd.print(" mb/h ");

    // Display time
    lcd.setCursor(0, 3);
    if(pressureChange >= 0.1f)
    {
        if(pressureChange >= 0.5f)
        {
            if(pressureChange >= 1.0f)
            {
                lcd.print("Warning        ");
            }
            else
            {
                lcd.print("Rising Rapidly ");
            }
        }
        else
        {
            lcd.print("Rising         ");
        }
    }
    else if(pressureChange <= -0.1f)
    {
        if(pressureChange <= -0.5f)
        {
            if(pressureChange <= -1.0f)
            {
                lcd.print("Warning        ");
            }
            else
            {
                lcd.print("Falling Rapidly");
            }
        }
        else
        {
            lcd.print("Falling        ");
        }
    }
    else
    {
        lcd.print("Steady         ");
    }

    //Serial.print("Approx. Altitude = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //Serial.println(" m");

    //Serial.print("Humidity = ");
    //Serial.print(bme.readHumidity());
    //Serial.println(" %");

    Serial.println();
}
