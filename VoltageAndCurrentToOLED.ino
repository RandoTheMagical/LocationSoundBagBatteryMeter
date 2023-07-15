/*
 * This is the code for my soundbag power monitor.
 * Modified for the INA219 power meter used in the pcb mounted version of my power supply
 * also testing github
 */


 /* Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * More pin layouts for other boards can be found here: https://github.com/miguelbalboa/rfid#pin-layout
 */
 
//#include <INA226_asukiaaa.h>
#include <Adafruit_INA219.h>
//#include <MFRC522.h> //this is for the RFID reader. Uncomment when ready to start deploying. Will need to install on surface to work there.


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "battery.h"

Adafruit_INA219 ina219;

//following defines are for RFID tag reader. uncomment when deploying rfid
/*
#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
*/

#define LED_BUILTIN 13
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define LOGO_HEIGHT   16
#define LOGO_WIDTH    30

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance. for rfid reader

/*
float Bat100Percent = 8.1; //8.4v is considered fully charged
//float Bat75Percent = ; //75 percent is higher than 50% and lower than 100%
float Bat50Percent = 7.64;
float Bat25Percent = 7.48;
float Bat0Percent = 6.6;  //6.64v is considered fully flat
*/

//percentages where the icon will show  relevent icon
int batteryDead = 10;
int battery25 = 30;
int battery50 = 50;
int battery75 = 70;
int batteryFull = 90;





float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage_V = 0;
float power_mW = 0;
float mAh = 0;
float energy_mWh = 0;

float BatterymAh = 0; //this is the measured capacity of the battery. The tested battery level is aprox 7000mAh / 48110mWh
//in the planned complete system, it will read from the RFID chip on the battery.
float BatterymWh = 53280; //possibly will use the Wh rating of the battery - 53.28 is the stated rating on the np-f970 battery
float BatterymWhRemaining = 53280; //currently set to same as BatteryWh, but in the monitoring system will most likely be read, and set from the RFID chip initially

unsigned long tick;
unsigned long lastread;
unsigned long previousMillis;
unsigned long previousWriteMillis;
unsigned long previousWrite;

//(Y/X *100 = P%)
//chargePercent = int(BatterymWhRemaining / BatterymWh *100);
int chargePercent;
int menuNumb = 1; //this number will indicate which display menu is selected
/*
menuNumb = 1: Default menu, shows multiple items
menuNumb = 2: Displays mWh left in battery
menuNumb = 3: Displays mWh drawn from battery
*/

const int interval = 200; //interval to update screen
const long writeInterval = 60000; //1 MINUTE
const float maxCharged = 8.3; //the maximum charge off a charger should be 8.4v. Using 8.3v gives a little error factor. this is used to decide if battery is fully charged.


void setup() {
 Serial.begin(115200);
 pinMode(2, INPUT_PULLUP); //reset counter button on pin 2
 pinMode(3, INPUT_PULLUP);

//  mfrc522.PCD_Init();    // Init MFRC522 rfid reader
//  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme

  
  //read data from RFID module
     //read batterymAh
     //read batteryWhRemaining
  loadInfo();//this function will read the battery info from the RFID chip, and also check the voltage on the battery

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
  int modeButtonState = digitalRead(3);

    if (resetButtonState == 0) //if button between pin2 and Gnd is pressed (input pulled low)
    {
      ResetPowerCount();//reset the project
    }
    
    /*
    the following if statement is for the menu select
    note that currently this has no delay in retriggering the change. 
    If you hold it down, it will cycle through the menu each iteration of the loop/
    which is currently untested, but will likely fly through.
    */
    if (modeButtonState == 0)
    {
      if(menuNumb <=2)
        {
          menuNumb++;
        }
        else
        {
          menuNumb = 1;
        }
    }

 // int16_t ma, mv, mw;
  if((currentMillis - previousWriteMillis) >= writeInterval)
    {
      previousWriteMillis = currentMillis;
      saveInfo();
    }

   if ((currentMillis - previousMillis) >= interval)
   {
    getinfo();
    chargePercent = int(BatterymWhRemaining / BatterymWh *100);
    Serial.println("charged Percent: "+ String(chargePercent));
   // float volts = 0.0;
    display.clearDisplay();
    
    if(menuNumb == 1)
    {
      display.setTextSize(1);
      display.println(String(int(mAh)) +"mAh");
      display.println(String(busvoltage) + "V," +String(current_mA) + "mA");
      display.println(String(BatterymWhRemaining) +"Wh Remaining");
       display.println(String(energy_mWh) +"mWh used so far");
      // display.println(BatterymWh);
    }
    else if(menuNumb == 2)
    {
      display.setTextSize(3);
      display.println(String(BatterymWhRemaining) +"remaining");
    }
    else if(menuNumb == 3 )
    {
      display.setTextSize(3);
      display.println(String(energy_mWh) +"mWh used");
    }
  
    if(chargePercent <= battery50)
    {
      //if the battery is less than 50%
      if(chargePercent <= batteryDead)//fully flat is 6.62v
      {
         display.println("0% WARNING");
           display.drawBitmap(98, 0, battery_0pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
      }
      else
      {
        if (chargePercent <= battery25)//if it is greater than the dead state, but less than battery25 percent
        {
             display.drawBitmap(98, 0, battery_25pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }
        else
        {
         //  display.println("50%");//otherwise, it is greater than the 25%, but still lower than the 50%, so show 50%
           display.drawBitmap(98, 0, battery_50pc, LOGO_WIDTH, LOGO_HEIGHT, 1);
        }
      }
    }
    else
    {
      //The battery is greater than 50%
      if(chargePercent >= batteryFull)//if the battery is greater than the full mark (which is 90%)
      {
        //show the full battery
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
  //check that the ID of the RFID hasn't changed
  //if (battery ID is stored id
  //  then save the current stats to 
  //open rfid write.
  //write current BatteryWhRemaining
  //close rfid write
  Serial.println("Battery eeprom updated after 1 minute");
  Serial.println("mWh used : " + String(energy_mWh) +" mWh");
  Serial.println("remaining: " + String(BatterymWhRemaining)+" mWh");
  Serial.println("Voltage  : " + String(busvoltage) +"V");
}

void loadInfo()
{
  float batteryLevelmV = ina219.getShuntVoltage_mV();
  //loads battery info from RFID eeprom
  //float BatterymWh - The total Mwh of the battery
  //float BatterymWhRemaining - The remaining power left in the battery
   if(batteryLevelmV >= maxCharged)//if the battery is full charged, resed the used mWh counter
   {
    ResetPowerCount();
    Serial.println("Watt hours reset to battery default");
   }
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
