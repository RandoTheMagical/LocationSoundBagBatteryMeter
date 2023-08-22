/*
 * This is the code for my soundbag power monitor.
 * Feel free to modify this program, improve it, customise it. Make it work for you.
 * If you like, Let me know how you went, I'd love to hear other peoples successes with it
 * 
 * hardware expected:
 * 128x32 SSD1306 i2c OLED display
 * INA219 i2c module on default i2c address
 */
#include <Adafruit_INA219.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "battery.h"

Adafruit_INA219 ina219;

#define LED_BUILTIN 13
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    30

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

float Bat100Percent = 8.1; //8.4v is considered fully charged
//float Bat75Percent = ; //75 percent is higher than 50% and lower than 100%
float Bat50Percent = 7.64;
float Bat25Percent = 7.48;
float Bat0Percent = 6.6;  //6.64v is considered fully flat

float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage_V = 0;
float power_mW = 0;
float mAh = 0;
float energy_mWh = 0;

float BatterymAh = 0; //this is the measured capacity of the battery.
float BatterymWh = 53280; //possibly will use the Wh rating of the battery - 53.28 is the stated rating on the np-f970 battery
float BatterymWhRemaining = 53280; //currently set to same as BatteryWh, but in the monitoring system will most likely be read, and set from the RFID chip initially

unsigned long tick;
unsigned long lastread;
unsigned long previousMillis;
unsigned long previousWriteMillis;
unsigned long previousWrite;

const int interval = 200; //interval to update screen
const long writeInterval = 60000; //1 MINUTE

void setup() {
 Serial.begin(115200);
 pinMode(2, INPUT_PULLUP); //reset counter button on pin 2

  //Start the OLED desplay:
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  //Start the INA219 current sensor:
  if (ina219.begin() != 0) {
    Serial.println("Failed to begin INA226");
  }
  
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  previousMillis, lastread = millis();
  previousWriteMillis, previousWrite = millis();
}

void loop() {

  unsigned long currentMillis = millis();
  int resetButtonState = digitalRead(2);
    if (resetButtonState == 0) //if button between pin2 and Gnd is pressed (input pulled low)
    {
      ResetPowerCount();//reset the project
    }

  int16_t ma, mv, mw;
  if((currentMillis - previousWriteMillis) >= writeInterval)
    {
      previousWriteMillis = currentMillis;
      saveInfo();
    }
   // Serial.println("CurrentMilis: " + String(currentMillis));
   // Serial.println("PreviousMillis: " + String(previousMillis));
   // Serial.println("previousWriteMillis: " + String(previousWriteMillis));
   // Serial.println("writeInterval: " + String(writeInterval));
   if ((currentMillis - previousMillis) >= interval)
   {
    getinfo();
    
   // float volts = 0.0;
    display.clearDisplay();
    
      display.println(String(int(mAh)) +"mAh");
      display.println(String(busvoltage) + "V," +String(current_mA) + "mA");
      display.println(String(BatterymWhRemaining) +"Wh Remaining");
       display.println(String(energy_mWh) +"mWh used so far");
   
  
    if(busvoltage <= Bat50Percent)//7.64 is 50 percent.
    {
      //if the battery is less than 50%
      if(busvoltage <= Bat0Percent)//fully flat is 6.62v
      {
         display.println("0% WARNING");
           display.drawBitmap(98, 0, battery_0pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
      }
      else
      {
        if (busvoltage <= Bat25Percent)
        {
         //  display.println("20%");
             display.drawBitmap(98, 0, battery_25pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }
        else
        {
         //  display.println("50%");
           display.drawBitmap(98, 0, battery_50pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }
      }
    }
    else
    {
      //The battery is greater than 50%
      if(busvoltage >= Bat100Percent)//fully charged is 8.4v
      {
       //  display.println("100%");
         display.drawBitmap(98, 0, battery_100pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
      }
      else //75 percent is higher than 50% and lower than 100%
      {
      //   display.println("75%");
           display.drawBitmap(98, 0, battery_75pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
      }
    }
      
   //   display.println("voltage:" +String(mw) + "mW");
      display.setCursor(0,0);
    display.display();
    delay(1000);
    previousMillis = currentMillis;
  } //end of if interval check
    
}//end of loop function

void getinfo()
{
  unsigned long newtime;
  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage_V = busvoltage + (shuntvoltage / 1000.0);
  newtime = millis();
  tick = newtime - lastread;
  mAh += current_mA * tick / 3600000.0;
  energy_mWh += power_mW * tick / 3600000.0;
  BatterymWhRemaining -= power_mW * tick / 3600000.0;//subtract the used power since last cycle
  lastread = newtime;
}

//saveInfo() will be called to write details to the RFID chip.
//for development puroposes, it may be used to save to some kind of eeprom or memory card until the RFID hardware arrives.
void saveInfo()
{
  //open rfid write.
  //write current BatteryWhRemaining
  //close rfid write
  Serial.println("Battery eeprom updated after 1 minute");
  Serial.println("Battery Watt Hour remaining: " + String(BatterymWhRemaining)+" mWh");
  Serial.println("mWh used: " + String(energy_mWh));
}

void ResetPowerCount()
{
        previousMillis, lastread = millis();
        previousWriteMillis, previousWrite = millis();
        power_mW = 0;
        mAh = 0;
        energy_mWh = 0;
        BatterymAh = 0; //this is the measured capacity of the battery.
        BatterymWhRemaining = BatterymWh; //currently set to same as BatteryWh, but in the monitoring system will most likely be read, and set from the RFID chip initially
}
