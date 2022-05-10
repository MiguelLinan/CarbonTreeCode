/*******************************************************
  Medidor ambiente
  modificada 8.8.2
  2021/07/21

  Author:
  Miguel Nunes Alves liñan da Silva

  Based on:
  "Name and authors of previous version"
*******************************************************/

//In the arduino code time is measured in milliseconds (ms)



#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>




//quick access variables:
unsigned long writeIntervalSD = 60000;         //variable for time between writings to the SD (Default)
unsigned long experimentDuration = 3600000;   //variable for the duration of the experiment (Default)(4294967295 is the max value and is set as infinite)
bool TroubleShooting = false;                 //skips initial messages and gives feedback through the serial port




//some of the pins:
int SDPin = 2;                              //SDreader pin
int NO2_In = A1;                            //NO2 sensor pin
int CO_In = A2;                          //CO sensor pin
int CO2_In = A3;                         //CO2 sensor pin
#define SPI_SS = SDPin;                     //define the default SS(slave select) as pin 2




//variables for the buttons:
int buttonsPin = A0;                        //pin of the LCD buttons
int buttonsValue;                           //analog signal from buttons
int buttonsSignal;                          //cleaned signal from the buttons
unsigned long debounceMillis;               //minimal amount of time in between button presses


//variables for the sensors:
float raw_NO2 = 0;                      //variable to store the value comming from the analog pin
float NO2_R = 0;                        //NO2 Sensor Resistance
float NO2_ppb = 0;                      //ppb NO2 value

int raw_CO2 = 0;                         //variable to store the value coming from the analog pin
double CO2_Volt;                         //voltage from the CO2 analog pin
int voltage_diference;                   //variable for calculations
float concentration;                     //variable to store the CO2 concentration
int raw_CO = 0;                          //variable to store the value coming from the analog pin
double CO_ppm;                           //variable to store the CO value

#define LENG 31                          //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value = 0;                      //define PM1.0 value of the air detector module
int PM2_5Value = 0;                     //define PM2.5 value of the air detector module
int PM10Value = 0;                      //define PM10 value of the air detector module



//Setup menus and display screens:
int setupMenu = 1;                       //variable for the setupMenus screen
unsigned menuDisplay = 1;                //variable for the displays during the experiment

int runSetup = 1;                        //variable for when the setup is occuring
unsigned long setupDuration;             //variable for the duration of the setup

int cursorPlace = 1;                     //variable for moving the cursor while editing time
unsigned long  amountTime;               //variable for how much time to add
unsigned  h, m;                          //variable for displaying the duration of the experiment to the LCD
unsigned  hI, mI, sI;                    //variable for displaying the time between writings to the SD and LCD



// LCD:
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel

byte warning[] = {    //warning "!!" character
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B00000,
  B01010
};

byte noteSD [] = {  //note write to the SD icon character
  B00000,
  B00010,
  B00111,
  B00010,
  B11000,
  B11100,
  B11100,
  B11100
};



//SD variables:
File CO2_data;                          //inside name for the CO2 Values.TXT file
File CO_data;                           //inside name for the CO Values.TXT file
File PM01_data;                         //inside name for the PM1.0 Values.TXT file
File PM2_5_data;                        //inside name for the PM2.5 Values.TXT file
File PM10_data;                         //inside name for the PM10.0 Values.TXT file
File NO2_data;                          //inside name for the NO2 Values.TXT file

unsigned long CO2_fileSize;                    //size in bytes of the SD file
unsigned long CO_fileSize;                    //size in bytes of the SD file
unsigned long PM01_fileSize;                    //size in bytes of the SD file
unsigned long PM2_5_fileSize;                    //size in bytes of the SD file
unsigned long PM10_fileSize;                    //size in bytes of the SD file
unsigned long NO2_fileSize;                    //size in bytes of the SD file

int writeNote = false;                     //variable for when to write a note too SD
long SDTimer;                              //variable for when to writeto SD (for millis function)
unsigned long updateTimer;



//auxiliary variables:
bool runOnce = false;                     //variable for only letting screen text be written once (and prevent static in the LCD)
bool runOnce2 = false;                    //another variable for only letting screen text be written once (and prevent static in the LCD)
int sizeShow = 0;

 
//////////////////////////////////////////////////////////////////////////////////////////////////void setup()////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  if (TroubleShooting == true) {
    Serial.begin(9600);
  }


  lcd.createChar(0, noteSD);             //create the warning character
  lcd.createChar(1, warning);            //create the warning character

  pinMode(SDPin, OUTPUT);                //necessary for SDreader to work
  pinMode (buttonsPin, INPUT);           //define A0 (buttons's pin) as input

  Serial.begin(9600);                    //start the Serial comunication with the PM2.5
  Serial.setTimeout(1500);               //if it does not start ignore

  analogReference(DEFAULT);              //set the default voltage for reference voltage


  // Beginning of setup
  lcd.begin(16, 2);                      //start the LCD library

  if (TroubleShooting == false) {
    lcd.clear();                   //clean the display before write
    lcd.print("**Carbon Tree**");                   //message to show
    lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
    lcd.print("Portugal 2021");                   //message to show
    delay(4000);                   //wait 4s

    lcd.clear();                           //clean the display before write
    lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
    lcd.print(" Ambient meter");                    //message to show
    delay(3000);                    //wait 3s


    lcd.clear();                           //clean the display before write
    lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
    lcd.write(byte(1));                    //display the warning character
    lcd.print("Wait 5 min");                   //message to show
    lcd.write(byte(1));                    //display the warning character
    lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
    lcd.print("to stabilize");                   //message to show
    delay(4000);                   //wait 4s
  }

  while (runSetup <= 4) {                //runs during the setup (which has 4 setupmenus)
    checkButton();

    //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» menu-1

    if (runSetup == 1) {
      while (runOnce == false) {          //only runs once so there's no static
        lcd.clear();
        lcd.setCursor(0, 0);                   //set the LCD cursor position first column(0) and first line(0)
        lcd.print("Experiment time");
        lcd.setCursor(0, 1);
        lcd.print("1-Endless 2-Set");
        runOnce = true;
        buttonsSignal = 0;                 //so it doesn't count the last buttons press when it shouldn't
      }


      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          runSetup = runSetup + 2;                       //go to menu 3
          runOnce = false;
          experimentDuration = 4294967295;                 //4294967295 is the max value and is set as infinity
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          runSetup ++;                                  //go to menu 2
          runOnce = false;
          experimentDuration;                           //set the experimentDuration as the default
          break;
      }
    }


    //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» menu-2


    if (runSetup == 2) {
      while (runOnce == false) {                 //only runs once so there's no static
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer setup:");
        runOnce = true;
        buttonsSignal = 0;                  //so it doesn't count the last buttons press when it shouldn't
      }
      if (millis() - debounceMillis >= 300) {
        debounceMillis = millis();
        lcd.setCursor(1, 3);
        h = (experimentDuration / 3600000);                     //converts milliseconds to hours
        m = (experimentDuration - (3600000 * h)) / 60000;       //converts milliseconds to minutes
        lcd.blink();                              //makes the LCD start blinking
        lcd.print(h);                             //shows the time in hours
        lcd.print("h:");
        lcd.print(m);                             //shows the time in minutes
        lcd.print("m   ");


        int lenghtH = log10(h);                   //counts the number of characters - 1
        int lenghtM = log10(m);                   //counts the number of characters - 1

        switch (cursorPlace) {
          case 1:         //cursor in h
            lcd.setCursor(lenghtH + 1, 3);          //set cursor to h positions
            amountTime = 3600000;                   //makes the time change in hours
            break;
          case 2:         //cursor in m
            lcd.setCursor(lenghtH + 4 + lenghtM , 3);          //set cursor to m positions
            amountTime = 60000;                                //makes the time change in minutes
            break;
        }

        if (experimentDuration < 60000) {              //minimal limit of 1m
          experimentDuration = 60000;
        }

        switch (buttonsSignal) {
          case 1:                                          //the most left button is being pressed
            cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));     //moves cursor to the left
            break;

          case 2:                                         //the  second button couting from the left is being pressed
            cursorPlace = cursorPlace + (1 - (cursorPlace >= 2));     //moves cursor to the right
            break;

          case 3:                                        //the down button is being pressed
            if (experimentDuration >= amountTime) {            //prevents variavle rollover
              experimentDuration = experimentDuration - amountTime;         //reduces the experiment duration
            }
            break;

          case 4:                                         //the up button is being pressed
            experimentDuration = experimentDuration + amountTime * ( 1 - ( experimentDuration >= 86400000));      //increases the experiment duration
            break;

          case 5:                                          //the right button is being pressed
            runSetup ++;                       //go to menu 3
            runOnce = false;
            lcd.noBlink();                     //makes the LCD stop blinking
            break;
        }
      }
    }


    //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» menu-3

    if (runSetup == 3) {
      while (runOnce == false) {          //only runs once so there's no static
        lcd.clear();
        lcd.setCursor(0, 0);                   //set the LCD cursor position first column(0) and first line(0)
        lcd.print("SaveSD frequency");
        lcd.setCursor(0, 1);
        lcd.print("1-Default  2-set");
        runOnce = true;
        buttonsSignal = 0;                  //so it doesn't count the last buttons press when it shouldn't
        delay(600);
      }

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          runSetup = runSetup + 2;                //go to menu 5 (because it doesn't exists it ends the setup)
          runOnce = false;
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          runSetup ++;                            //go to menu 4
          runOnce = false;
          break;
      }
    }


    //»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»» menu-4

    if (runSetup == 4) {
      while (runOnce == false) {          //only runs once so there's no static
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Writing interval");
        runOnce = true;
        buttonsSignal = 0;                 //so it doesn't count the last buttons press when it shouldn't
      }

      if (millis() - debounceMillis >= 300) {
        debounceMillis = millis();
        lcd.setCursor(1, 3);
        hI = (writeIntervalSD / 3600000);                                 //converts milliseconds to hours
        mI = (writeIntervalSD - (3600000 * hI)) / 60000;                  //converts milliseconds to minutes
        sI = ((writeIntervalSD - (3600000 * hI)) - (mI * 60000)) / 1000;  //converts milliseconds to seconds
        lcd.blink();                              //makes the LCD start blinking
        lcd.print(hI);                            //shows the interval time in hours
        lcd.print("h:");
        lcd.print(mI);                            //shows the interval time in minutes
        lcd.print("m:");
        lcd.print(sI);                            //shows the interval time in seconds
        lcd.print("s     ");

        int lenghtHI = log10(hI);                  //counts the number of characters - 1
        int lenghtMI = log10(mI);                  //counts the number of characters - 1
        int lenghtSI = log10(sI);                  //counts the number of characters - 1

        switch (cursorPlace) {
          case 1:            //cursor in h
            lcd.setCursor(lenghtHI + 1, 3);                                 //set cursor to h positions
            amountTime = 3600000;
            break;
          case 2:            //cursor in m
            lcd.setCursor(lenghtHI + 4 + lenghtMI , 3);                     //set cursor to m positions
            amountTime = 60000;
            break;

          case 3:            //cursor in s
            lcd.setCursor(lenghtHI + lenghtMI + lenghtSI + 7, 3);                     //set cursor to s positions
            amountTime = 1000;
            break;
        }

        if (writeIntervalSD < 1000) {                   //minimal limit of 1s
          writeIntervalSD = 1000;
        }

        switch (buttonsSignal) {
          case 1:                                          //the most left button is being pressed
            cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));          //moves cursor to the left
            break;

          case 2:                                         //the  second button couting from the left is being pressed
            cursorPlace = cursorPlace + (1 - (cursorPlace >= 3));          //moves cursor to the right
            break;

          case 3:                                        //the down button is being pressed
            if (writeIntervalSD >= amountTime) {
              writeIntervalSD = writeIntervalSD - amountTime;                 //reduces the interval between data saves
            }
            break;

          case 4:                                         //the up button is being pressed
            writeIntervalSD = writeIntervalSD + amountTime;                   //increases the interval between data saves
            break;

          case 5:                                          //the right button is being pressed
            runSetup ++;
            runOnce = false;
            lcd.noBlink();                     //makes the LCD stop blinking
            break;
        }
      }
    }
  }

  if (SD.begin()) {                                      //checking if SDreader and SDcard are connected and start the SD library
    hI = (writeIntervalSD / 3600000);                                 //converts milliseconds to hours
    mI = (writeIntervalSD - (3600000 * hI)) / 60000;                  //converts milliseconds to minutes
    sI = ((writeIntervalSD - (3600000 * hI)) - (mI * 60000)) / 1000;  //converts milliseconds to seconds


    CO2_data = SD.open("CO2 Values.TXT", FILE_WRITE);
    CO2_data.print("Beginning of experience - write frequency: ");
    CO2_data.print(hI);
    CO2_data.print(":");
    CO2_data.print(mI);
    CO2_data.print(":");
    CO2_data.println(sI);
    CO2_fileSize = CO2_data.size();
    CO2_data.close();

    CO_data = SD.open("CO Values.TXT", FILE_WRITE);
    CO_data.print("Beginning of experience - write frequency: ");
    CO_data.print(hI);
    CO_data.print(":");
    CO_data.print(mI);
    CO_data.print(":");
    CO_data.println(sI);
    CO_fileSize = CO_data.size();
    CO_data.close();

    PM01_data = SD.open("PM1.0 Values.TXT", FILE_WRITE);
    PM01_data.print("Beginning of experience - write frequency: ");
    PM01_data.print(hI);
    PM01_data.print(":");
    PM01_data.print(mI);
    PM01_data.print(":");
    PM01_data.println(sI);
    PM01_fileSize = PM01_data.size();
    PM01_data.close();

    PM2_5_data = SD.open("PM2.5 Values.TXT", FILE_WRITE);
    PM2_5_data.print("Beginning of experience - write frequency: ");
    PM2_5_data.print(hI);
    PM2_5_data.print(":");
    PM2_5_data.print(mI);
    PM2_5_data.print(":");
    PM2_5_data.println(sI);
    PM2_5_fileSize = PM2_5_data.size();
    PM2_5_data.close();

    PM10_data = SD.open("PM10.0 Values.TXT", FILE_WRITE);
    PM10_data.print("Beginning of experience - write frequency: ");
    PM10_data.print(hI);
    PM10_data.print(":");
    PM10_data.print(mI);
    PM10_data.print(":");
    PM10_data.println(sI);
    PM10_fileSize = PM10_data.size();
    PM10_data.close();

    NO2_data = SD.open("NO2 Values.TXT", FILE_WRITE);
    NO2_data.print("Beginning of experience - write frequency: ");
    NO2_data.print(hI);
    NO2_data.print(":");
    NO2_data.print(mI);
    NO2_data.print(":");
    NO2_data.println(sI);
    NO2_fileSize = NO2_data.size();
    NO2_data.close();
  }



  lcd.clear();
  setupDuration = millis();            //save duration of setup
  SDTimer = -writeIntervalSD;          //necessary for all sensores to start as soon as the void loop starts
}


//////////////////////////////////////////////////////////////////////////////////////////////////void loop()////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  checkButton();
  if (millis() > debounceMillis + 150) {          //checks buttons every 150ms
    debounceMillis = millis();

    switch (buttonsSignal) {
      case 0:                                         //no buttons is being pressed (divider can should be changed if button reading is wrong)
        break;
      case 1:                                         //the most left button is being pressed
        break;

      case 2:                                         //the  second button couting from the left is being pressed
        break;

      case 3:                                         //the down button is being pressed
        menuDisplay = menuDisplay + ( 1 - ( menuDisplay >= 4 ));   //scroll down
        runOnce = false;
        break;

      case 4:                                         //the up button is being pressed
        menuDisplay = menuDisplay - ( 1 - ( menuDisplay <= 1 ));   //scroll up
        runOnce = false;
        break;

      case 5:                                         //the right button is being pressed
        writeNote = true;                             //write note to "CTdata.TXT" file
        runOnce = false;
        break;
    }
  }

  if (experimentDuration >= (millis() - setupDuration )) {               //run while experiment isn't over

    if (millis() - SDTimer > writeIntervalSD) {                           //runs when the write interval has passed
      SDTimer = millis();                          //load the millis time to the SDTimer
      runOnce = false;

      raw_NO2 = analogRead(NO2_In);          //read the analog in value
      float NO2_Volt = raw_NO2 / 409.2;     //conversion formula
      NO2_R = 22000 / ((5 / NO2_Volt) - 1);  //find sensor resistance from No2_Volt, using 5V input & 22kOhm load resistor
      NO2_ppb = (.000008 * NO2_R - .0194) * 1000; //convert Rs to ppb (parts per billion) concentration of NO2

      raw_CO2 = analogRead(CO2_In);          //read the analog in value
      CO2_Volt = raw_CO2 * (5000 / 1024.0); //conversion formula

      raw_CO = analogRead(CO_In);          //read the analog in value
      CO_ppm = 3.027 * exp(1.0698 * (raw_CO * (5.0 / 1024))); //conversion formula

      if (Serial.find(0x42)) {            //look for the 0x42 in the Serial port
        Serial.readBytes(buf, LENG);      //load the buf with the values

        if (buf[0] == 0x4d) {
          if (checkValue(buf, LENG)) {
            PM01Value = transmitPM01(buf);   //count PM1.0 value of the air detector module
            PM2_5Value = transmitPM2_5(buf);  //count PM2.5 value of the air detector module
            PM10Value = transmitPM10(buf);   //count PM10 value of the air detector module
          }
        }
      }
      CO2_data = SD.open("CO2 Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        CO2_data.println("-----------------------------");        //note that'll be written to SD file

        runOnce = false;
      }
      CO2_data.println(concentration);                                   //save CO2 concentration to the "CO2 Values.TXT"
      CO2_fileSize = CO2_data.size();                               //read file size
      CO2_data.close();                                         //close file after writing to prevent corruption

      CO_data = SD.open("CO Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        CO_data.println("-----------------------------");        //note that'll be written to SD file
        
        runOnce = false;
      }
      CO_data.println(CO_ppm);                                   //save CO concentration to the "CO Values.TXT"
      CO_fileSize = CO_data.size();                               //read file size
      CO_data.close();                                         //close file after writing to prevent corruption

      PM01_data = SD.open("PM1.0 Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        PM01_data.println("-----------------------------");        //note that'll be written to SD file
        
        runOnce = false;
      }
      PM01_data.println(PM01Value);                                   //save PM 1.0 concentration to the "PM1.0 Values.TXT"
      PM01_fileSize = PM01_data.size();                               //read file size
      PM01_data.close();                                         //close file after writing to prevent corruption

      PM2_5_data = SD.open("PM2.5 Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        PM2_5_data.println("-----------------------------");        //note that'll be written to SD file

        runOnce = false;
      }
      PM2_5_data.println(PM01Value);                                   //save PM2.5 concentration to the "PM2.5 Values.TXT"
      PM2_5_fileSize = PM2_5_data.size();                               //read file size
      PM2_5_data.close();                                         //close file after writing to prevent corruption

      PM10_data = SD.open("PM10.0 Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        PM10_data.println("-----------------------------");        //note that'll be written to SD file

        runOnce = false;
      }
      PM10_data.println(PM10Value);                                   //save PM10.0 concentration to the "PM10.0 Values.TXT"
      PM10_fileSize = PM10_data.size();                               //read file size
      PM10_data.close();                                         //close file after writing to prevent corruption

      NO2_data = SD.open("NO2 Values.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        NO2_data.println("-----------------------------");        //note that'll be written to SD file
        writeNote = false;                                                  //reset note warning (note has been written)
        runOnce = false;
      }
      NO2_data.println(PM10Value);                                   //save NO2 concentration to the "NO2 Values.TXT"
      NO2_fileSize = NO2_data.size();                               //read file size
      NO2_data.close();                                         //close file after writing to prevent corruption
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ display-1

    if ((menuDisplay == 1) & (runOnce == false))  {               //only runs once there's no static and when it's on the menu 1
      runOnce = true;
      lcd.clear();                                    //clean the display before write
      if (CO2_Volt == 0) {
        // print the results to the lcd
        lcd.print("CO2: ");                             //message to show
        lcd.print("Fault");                             //message to show
      }
      else if (CO2_Volt < 400) {
        // print the results to the lcd
        lcd.print("CO2: ");                             //message to show
        lcd.print("preheating");                        //message to show
      }
      else {
        voltage_diference = CO2_Volt - 400;
        concentration = voltage_diference * (50.0 / 16.0); //auxiliar calculation
        lcd.print("CO2: ");                             //message to show
        lcd.print(concentration);                       // print the results to the lcd
        lcd.print(" ppm");                               //message to show
      }

      lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
      lcd.print("CO: ");                              //message to show
      lcd.print(CO_ppm);                              //print the results to the lcd
      lcd.print(" ppm");                               //message to show
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ display-2

    if ((menuDisplay == 2) & (runOnce == false))  {               //only runs once there's no static and when it's on the menu 2
      runOnce = true;
      lcd.clear();                                    //clean the display before write



      lcd.print("PM1.0: ");                           //message to show
      lcd.print(PM01Value);                           //print the results to the lcd
      lcd.print(" ug/m3");                             //message to show

      lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
      lcd.print("PM2.5: ");                           //message to show
      lcd.print(PM2_5Value);                          //print the results to the lcd
      lcd.print(" ug/m3");                             //message to show
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ display-3

    if ((menuDisplay == 3) & (runOnce == false))  {                //only runs once there's no static and when it's on the menu 3
      runOnce = true;
      lcd.clear();                                    //clean the display before write



      lcd.print("PM10: ");                            //message to show
      lcd.print(PM10Value);                           //print the results to the lcd
      lcd.print(" ug/m3");                            //message to show

      lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
      lcd.print("NO2: ");                             //message to show
      lcd.print(NO2_ppb);                             //print the results to the lcd
      lcd.print(" ppb");                              //message to show
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ display-4

    if ((menuDisplay == 4) && ((runOnce == false) || (millis() > (60000 + updateTimer))))  {     //only runs every minute so there's no static and when it's on the menu 4
      updateTimer = millis();
      runOnce = true;
      lcd.clear();                                    //clean the display before write
      lcd.setCursor(0, 0);                            //set the LCD cursor position first column(0) and second line(0)
      if (SD.begin()) {                 //only runs if SD is connected
        if (experimentDuration != 4294967295) {               //only runs if experiment duration is not infinite
          lcd.print("Ends in: ");                       //message to show
          lcd.setCursor(9, 0);
          int hTimer = ((experimentDuration - millis() + setupDuration) / 3600000);                       //converts milliseconds to hours
          int mTimer = ((experimentDuration - millis() + setupDuration) - (3600000 * hTimer)) / 60000;    //converts milliseconds to minutes
          lcd.print(hTimer);                       //message to show
          lcd.print("h");                       //message to show
          lcd.print(mTimer);                       //message to show
          lcd.print("m");                       //message to show
        }
        else {
          lcd.print("Endless mode");                       //message to show
        }
        if (writeNote) {

          lcd.setCursor(15, 0);                       //set the LCD cursor position column 15(16) and first line(0)
          lcd.write(byte(0));                         //indicates that it needs to write a note("---------------"), shows icon
        }

        lcd.setCursor(0, 1);                           //set the LCD cursor position first column(0) and second line(1)
        switch (sizeShow) {
          case 1:
            sizeShow += 1;
            lcd.print("CO2 file:");                        //message to show
            lcd.print(CO2_fileSize);                            //print file's size in bytes to the lcd
            break;

          case 2:
            sizeShow += 1;
            lcd.print("CO file:");                        //message to show
            lcd.print(CO_fileSize);                            //print file's size in bytes to the lcd
            break;

          case 3:
            sizeShow += 1;
            lcd.print("PM1.0 size:");                        //message to show
            lcd.print(PM01_fileSize);                            //print file's size in bytes to the lcd
            break;

          case 4:
            sizeShow += 1;
            lcd.print("PM2.5 size:");                        //message to show
            lcd.print(PM2_5_fileSize);                            //print file's size in bytes to the lcd
            break;


          case 5:
            sizeShow += 1;
            lcd.print("PM10 size:");                        //message to show
            lcd.print(PM10_fileSize);                            //print file's size in bytes to the lcd
            break;


          case 6:
            sizeShow = 0;
            lcd.print("NO2 size:");                        //message to show
            lcd.print(NO2_fileSize);                            //print file's size in bytes to the lcd
            break;

        }
      }

      else {
        lcd.print("SD not connected");                 //message to show
      }
    }
  }

  else {

    lcd.clear();
    lcd.setCursor(3, 0);                    //set the LCD cursor position column 4(3) and first line(o)
    lcd.print ("Done!!");                      //message to show

    CO2_data = SD.open("CO2 Values.TXT", FILE_WRITE);            //open file for writing
    CO2_data.println("End of experiment");       //message to save
    CO2_data.close();                           //closes file


    CO_data = SD.open("CO Values.TXT", FILE_WRITE);            //open file for writing
    CO_data.println("End of experiment");       //message to save
    CO_data.close();                           //closes file

    PM01_data = SD.open("PM1.0 Values.TXT", FILE_WRITE);            //open file for writing
    PM01_data.println("End of experiment");       //message to save
    PM01_data.close();                           //closes file

    PM2_5_data = SD.open("PM2.5 Values.TXT", FILE_WRITE);            //open file for writing
    PM2_5_data.println("End of experiment");       //message to save
    PM2_5_data.close();                           //closes file

    PM10_data = SD.open("PM10.0 Values.TXT", FILE_WRITE);            //open file for writing
    PM10_data.println("End of experiment");       //message to save
    PM10_data.close();                           //closes file

    NO2_data = SD.open("NO2 Values.TXT", FILE_WRITE);            //open file for writing
    NO2_data.println("End of experiment");       //message to save
    NO2_data.close();                           //closes file


    while (true) {                        //stops everything
    }
  }
}




//////////////////////////////////////////////////////////////////////////////// subroutines for PM2.5 sensor and button anolog function ///////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;

  for (int i = 0; i < (leng - 2); i++) {
    receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if (receiveSum == ((thebuf[leng - 2] << 8) + thebuf[leng - 1])) //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3] << 8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5] << 8) + thebuf[6]); //count PM2.5 value of the air detector module
  return PM2_5Val;
}

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7] << 8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}

int checkButton() {
  buttonsValue = analogRead(buttonsPin);     //output from the LCD buttons (divisor pode ser alterado se a leitura dos botões estiver com descrepãncias)
  if (buttonsValue > 190) {                //interpreting the analog buttons signal
    if (buttonsValue > 370) {
      if (buttonsValue > 510) {
        if (buttonsValue > 720) {
          if (buttonsValue > 900) {
            buttonsSignal = 0;
          }
          else {
            buttonsSignal = 1;
          }
        }
        else {
          buttonsSignal = 2;
        }
      }
      else {
        buttonsSignal = 3;
      }
    }
    else {
      buttonsSignal = 4;
    }
  }
  else {
    buttonsSignal = 5;
  }
}
