/*
 * This is the code for my soundbag power monitor.
 * Modified for the INA219 power meter used in the pcb mounted version of my power supply
 * also testing github
 */


//#include <INA226_asukiaaa.h>
#include <Adafruit_INA219.h>


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "battery.h"

//const uint16_t ina226calib = INA226_asukiaaa::calcCalibByResistorMilliOhm(100); // Max 5120 milli ohm
// const uint16_t ina226calib = INA226_asukiaaa::calcCalibByResistorMicroOhm(2000);
//INA226_asukiaaa voltCurrMeter(INA226_ASUKIAAA_ADDR_A0_GND_A1_GND, ina226calib);
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

void setup() {
 Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.display();
  delay(2000);
  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
//  display.setCursor(0, 10);
//  display.display(); 
  if (ina219.begin() != 0) {
    Serial.println("Failed to begin INA226");
  }
}

void loop() {
  int16_t ma, mv, mw;
  float volts = 0.0;
  display.clearDisplay();
  
    volts = ina219.getBusVoltage_V();
    display.println(String(volts) + "V");

/*
  if (voltCurrMeter.readMW(&mw) == 0) {
    //volts = (float)mv / 1000;
    display.println(String(-mw) + "mW");
  } else {
    display.println("Cannot read watts.");
  }
*/
    ma = ina219.getCurrent_mA();
    display.println(String(ma) + "mA");
 

  if(volts <= Bat50Percent)//7.64 is 50 percent.
  {
    //if the battery is less than 50%
    if(volts <= Bat0Percent)//fully flat is 6.62v
    {
       display.println("0% WARNING");
         display.drawBitmap(98, 0, battery_0pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
    }
    else
    {
      if (volts <= Bat25Percent)
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
    if(volts >= Bat100Percent)//fully charged is 8.4v
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
}
