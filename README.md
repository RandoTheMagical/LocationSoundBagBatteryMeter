# LocationSoundBagBatteryMeter
This is an arduino project that monitors a battery input, displays the current voltage / battery level, and current usage on an OLED display.
It will run on an arduino - I've used a pro micro during development.

  hardware expected:
    128x32 SSD1306 i2c OLED display
    INA219 i2c module on default i2c address

Note: this code has been branched from an early iteration of my code. It should work,but I need to rebuild a test platform with the arduino to confirm, and fine tune the project. I will try and get to this when I have a chance. In the mean time, let me know if you run into any issues.
