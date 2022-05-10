/*******************************************************
  Medidor ambiente
  modificada 8.2
  2021/04/02

  Author:
  Miguel Nunes Alves liñan da Silva
  
  Based on:
  "Name and authors of previous version"
*******************************************************./
  OBS:
  - changed "StationData = SD.open("CTdata.TXT", FILE_WRITE);" to " SD.open("CTdata.TXT", FILE_WRITE);" (shouldn't cause any problems).
  - made it easier to set a default experiment duration.
  - having a low frequency of saves to the SD, like 1s, makes scrolling the displays more difficult. (potencial solutuion: increase the minimal limit) 
  
 Improvements to come:
 1- use millis on the interpretation of pressing a buttons intead of "delay(150)"
 2- make it possible to end an experiment while on infinite mode(putting and end message in the file), without having to turn it off.                        


  /*----------------------------------------------------//
  //SD conection:                                       //
  //CS- pin2                                            //
  //mosi- pin11                                         //
  //miso- pin12                                         //
  //SCK- pin13                                          //
  //----------------------------------------------------*/


  //In the arduino code time is measured in milliseconds (ms)



#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>




//quick access variables:
unsigned long writeIntervalSD = 60000;         //variable for time between writings to the SD (Default)
unsigned long experimentDuration = 3600000;   //variable for the duration of the experiment (Default)(4294967295 is the max value and is set as infinite)




//some of the pins:
#define SPI_SS = SDPin;                     //define the default SS(slave select) as pin 2
int SDPin = 2;                              //SDreader pin
int NO2_In = A1;                            //NO2 sensor pin
int CO_In = A2;                          //CO sensor pin
int CO2_In = A3;                         //CO2 sensor pin



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
unsigned menuDisplay = 1;                           //oidcbçsebçvoisvoisnevoise

int runSetup = 1;                        //variable for when the setup is occuring
unsigned long setupDuration;

int cursorPlace = 1;
unsigned long  amountTime;
unsigned  h, m;                             //variable for displaying the time between writings to the SD
unsigned  hI, mI, sI;                            //intervalo ------------------------------------------------------------------



// LCD:
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel

byte warning[] = {
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B01010,
  B00000,
  B01010
};

byte noteSD [] = {
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
File StationData;
unsigned long fileSize;                    //size in bytes of the SD file
int writeNote = false;                       //variable for when to write a note too SD
unsigned long SDTimer;                     //variable for when to writeto SD (for millis function)
unsigned long updateTimer;



//auxiliary variables:
bool runOnce = false;                     //variable for only letting screen text be written once (and prevent static)   ----
bool runOnce2 = false;                    //another variable for only letting screen text be written once (and prevent static)



//////////////////////////////////////////////////////////////////////////////////////////////////void setup()////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  lcd.createChar(0, noteSD);   // ------------------------------------------------------
  lcd.createChar(1, warning);

  pinMode(SDPin, OUTPUT);                //necessary for SDreader to work
  pinMode (buttonsPin, INPUT);           //define A0 (buttons's pin) as input

  Serial.begin(9600);                    //start the Serial comunication with the PM2.5
  Serial.setTimeout(1500);               //if it does not start ignore

  analogReference(DEFAULT);              //set the default voltage for reference voltage


  // Beginning of setup
  lcd.createChar(0, noteSD);
  lcd.begin(16, 2);                      //start the LCD library

  lcd.clear();                           //clean the display before write
    lcd.print("**Carbon Tree**");         //message to show
    lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
    lcd.print("Portugal 2021");         //message to show
    delay(4000);       
    
    lcd.clear();                           //clean the display before write
    lcd.setCursor(0, 0); 
    lcd.print(" Ambient meter");  
    delay(3000);

    
    lcd.clear();  
    lcd.setCursor(0, 0);
    lcd.write(byte(1)); 
    lcd.print("Wait 5 min");  
    lcd.write(byte(1)); 
    lcd.setCursor(0, 1); 
    lcd.print("to stabilize");  
    delay(4000); 

  while (runSetup <= 4) {


    ////////////////////////////////////////////////////////////////////////////menu-1

    if (runSetup == 1) {
      while (runOnce == false) {
        lcd.clear();
        lcd.setCursor(0, 0);                   //set the LCD cursor position first column(0) and first line(0)
        lcd.print("Experiment time");
        lcd.setCursor(0, 1);
        lcd.print("1-Endless 2-Set");
        runOnce = true;
        buttonsSignal = 0;
      }


      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          runSetup = runSetup + 2;
          runOnce = false;
          experimentDuration = 4294967295;                 //4294967295 is the max value and is set as infinity 
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          runSetup ++;
          runOnce = false;
          experimentDuration = 3600000;
          break;
      }
    }


    ////////////////////////////////////////////////////////////////////////////menu-2


    if (runSetup == 2) {
      while (runOnce == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer setup:");
        runOnce = true;
        buttonsSignal = 0;

      }
      lcd.setCursor(1, 3);
      h = (experimentDuration / 3600000);
      m = (experimentDuration - (3600000 * h)) / 60000;
      lcd.blink();
      lcd.print(h);
      lcd.print("h:");
      lcd.print(m);
      lcd.print("m   ");

      int lenghtH = log10(h);
      int lenghtM = log10(m);

      switch (cursorPlace) {
        case 1:                         //h
          lcd.setCursor(lenghtH + 1, 3);
          amountTime = 3600000;
          break;
        case 2:                           //hm
          lcd.setCursor(lenghtH + 4 + lenghtM , 3);                     //+ 4reasonn,,
          amountTime = 60000;
          break;
      }

      if (experimentDuration < 60000) {
        experimentDuration = 60000;
      }

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed

          cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          cursorPlace = cursorPlace + (1 - (cursorPlace >= 2));
          break;

        case 3:                                        //the down button is being pressed
          if (experimentDuration >= amountTime) {
            experimentDuration = experimentDuration - amountTime;         //3  subtrack -----
          }


          break;

        case 4:                                         //the up button is being pressed
          experimentDuration = experimentDuration + amountTime * ( 1 - ( experimentDuration >= 86400000));       //4    add

          break;
        case 5:                                          //the right button is being pressed
          runSetup ++;
          runOnce = false;
          lcd.noBlink();
          break;
      }
    }


    ////////////////////////////////////////////////////////////////////////////menu-3

    if (runSetup == 3) {
      while (runOnce == false) {
        lcd.clear();
        lcd.setCursor(0, 0);                   //set the LCD cursor position first column(0) and first line(0)
        lcd.print("SaveSD frequency");
        lcd.setCursor(0, 1);
        lcd.print("1-Default  2-set");
        runOnce = true;
        buttonsSignal = 0;
      }

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          runSetup = runSetup + 2;
          runOnce = false;
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          runSetup ++;
          runOnce = false;
          break;
      }
    }


    ////////////////////////////////////////////////////////////////////////////menu-4

    if (runSetup == 4) {
      while (runOnce == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Writing interval");
        runOnce = true;
        buttonsSignal = 0;
      }
      lcd.setCursor(1, 3);
      hI = (writeIntervalSD / 3600000);
      mI = (writeIntervalSD - (3600000 * hI)) / 60000;
      sI = ((writeIntervalSD - (3600000 * hI)) - (mI * 60000)) / 1000;
      lcd.blink();
      lcd.print(hI);
      lcd.print("h:");
      lcd.print(mI);
      lcd.print("m:");
      lcd.print(sI);
      lcd.print("s     ");

      int lenghtHI = log10(hI);
      int lenghtMI = log10(mI);
      int lenghtSI = log10(sI);

      switch (cursorPlace) {
        case 1:                         //h
          lcd.setCursor(lenghtHI + 1, 3);
          amountTime = 3600000;
          break;
        case 2:                           //hm
          lcd.setCursor(lenghtHI + 4 + lenghtMI , 3);                     //+ 4reasonn,,
          amountTime = 60000;
          break;

        case 3:                           //hm
          lcd.setCursor(lenghtHI + lenghtMI + lenghtSI + 7, 3);                     //+ 7reasonn,,
          amountTime = 1000;
          break;
      }

      if (writeIntervalSD < 1000) {
        writeIntervalSD = 1000;
      }

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          cursorPlace = cursorPlace + (1 - (cursorPlace >= 3));


          break;

        case 3:                                        //the down button is being pressed
          if (writeIntervalSD >= amountTime) {
              writeIntervalSD = writeIntervalSD - amountTime;                 //3  subtrack more a---
          }
          break;

        case 4:                                         //the up button is being pressed
          writeIntervalSD = writeIntervalSD + amountTime; //4    add more

          break;

        case 5:                                          //the right button is being pressed
          runSetup ++;
          runOnce = false;
          lcd.noBlink();
          break;
      }
    }

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
    delay(150);
  }

  if (SD.begin()) {                                      //checking if SDreader and SDcard are connected and start the SD library
    SD.open("CTdata.TXT", FILE_WRITE);
    StationData.print("Beginning of experiment - write frequency: ");
    StationData.print(hI);
    StationData.print(":");
    StationData.print(mI);
    StationData.print(":");
    StationData.println(sI);
    fileSize = StationData.size();
    StationData.close();
  }

  lcd.clear();
  setupDuration = millis();
}


//////////////////////////////////////////////////////////////////////////////////////////////////void loop()////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  if (millis() > debounceMillis + 100) {
    debounceMillis = millis();
    buttonsValue = analogRead(buttonsPin);     //output from the LCD buttons (divisor pode ser alterado se a leitura dos botões estiver com descrepãncias)-------------
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
    switch (buttonsSignal) {
      case 0:                                         //no buttons is being pressed (divider can should be changed if button reading is wrong)
        break;
      case 1:                                         //the most left button is being pressed
        //SDWriting = true;                                //enables writing to SD
        break;

      case 2:                                         //the  second button couting from the left is being pressed
        //SDWriting = false;                                //enables writing to SD
        break;

      case 3:                                         //the down button is being pressed
        menuDisplay = menuDisplay + ( 1 - ( menuDisplay >= 4 ));   //set writeinter--------------------------
        runOnce = false;
        break;

      case 4:                                         //the up button is being pressed
        menuDisplay = menuDisplay - ( 1 - ( menuDisplay <= 1 ));                //set write interval to intervalMode2(1m in this case)
        runOnce = false;
        break;

      case 5:                                         //the right button is being pressed
        writeNote = true;
        runOnce = false;
        break;
    }
  }

  if (experimentDuration >= (millis() - setupDuration )) {

    if (millis() - SDTimer > writeIntervalSD) {
      SDTimer = millis();      //load the millis time to the SDTimer
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
            PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
            PM2_5Value = transmitPM2_5(buf);//count PM2.5 value of the air detector module
            PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
          }
        }
      }
      
      SD.open("CTdata.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        StationData.println("-----------------------------");        //note that'll be written to SD file
        writeNote = false;                                                  //reset node warning (note has been written)
        runOnce = false;

      }
      StationData.println("");
      StationData.println(concentration);                                   //SD write CO2 concentration
      StationData.println(CO_ppm);
      StationData.println(PM01Value);
      StationData.println(PM2_5Value);
      StationData.println(PM10Value);
      StationData.println(NO2_ppb);
      fileSize = StationData.size();                               //read file size
      StationData.close();                                         //close file after writing to prevent corruption
    }



    if ((menuDisplay == 1) & (runOnce == false))  {
      runOnce = true;
      lcd.clear();                                    //clean the display before write
      if (CO2_Volt == 0) {
        // print the results to the lcd
        lcd.print("CO2: ");                             //message to show
        lcd.print("Fault");                             //message to show
      }
      if ((CO2_Volt < 400) && (CO2_Volt != 0) ) {               //////////////////////thisssss
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

    if ((menuDisplay == 2) & (runOnce == false))  {
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

    if ((menuDisplay == 3) & (runOnce == false))  {
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



    if ((menuDisplay == 4) && ((runOnce == false) || (millis() > (60000 + updateTimer))))  {
      updateTimer = millis();
      runOnce = true;
      lcd.clear();                                    //clean the display before write
      lcd.setCursor(0, 0);                            //set the LCD cursor position first column(0) and second line(0)
      if (SD.begin()) {                 //che
        lcd.print("Ends in: ");                       //message to show



        if (experimentDuration != 4294967295) {
          lcd.setCursor(9, 0);
          int hTimer = ((experimentDuration - millis() + setupDuration) / 3600000);
          int mTimer = ((experimentDuration - millis() + setupDuration) - (3600000 * hTimer)) / 60000;
          lcd.print(hTimer);                       //message to show
          lcd.print("h");                       //message to show
          lcd.print(mTimer);                       //message to show
          lcd.print("m");                       //message to show

        }
        if (writeNote) {

          lcd.setCursor(15, 0);
          lcd.write(byte(0));                           //indicates that it needs to writea note("---------------")
        }

        lcd.setCursor(0, 1);                           //set the LCD cursor position first column(0) and second line(1)
        lcd.print("filesize:");                        //message to show
        lcd.print(fileSize);                            //print file's size in bytes to the lcd
      }

      else {                                                  //delete
        lcd.print("SD not connected");                 //message to show
      }                                               //delete
    }// end do display 4
  }

  else {
    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print ("Done!!");
    SD.open("CTdata.TXT", FILE_WRITE);
    StationData.println("End of experiment");
    StationData.close();
    while (true) {

    }
  }
}




//////////////////////////////////////////////////////////////////////////////// subroutines for PM2.5 sensor///////////////////////////////////////////////
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
