/*******************************************************
  Medidor ambiente
  modificada 5.0
  2021/03/30
********************************************************/


/*----------------------------------------------------//
  //SD conection:                                       //
  //CS- pin2                                            //
  //mosi- pin11                                         //
  //miso- pin12                                         //
  //SCK- pin13                                          //
  //----------------------------------------------------*/


//have in mind that the C02 sensor only refreshes every 2m//

//it's easy to change the time inbetween by changing the values and letters in (~~)//

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
//some of the pins:
#define SPI_SS = SDPin;                     //define the default SS(slave select) as pin 2

int buttonsPin = A0;                         //pin of the LCD buttons
int buttonsValue;                           //analog signal from buttons(5 stage voltage divider)
int buttonsSignal;                          //cleaned signal from the buttons

int SDPin = 2;                              //SDreader pin
int NO2_In = A1;                            //NO2 sensor pin
int CO_In = A2;                          //CO sensor pin
int CO2_In = A3;                         //CO2 sensor pin
//LCD pins below


File StationData;
unsigned long fileSize;                    //size in bytes of the SD file
int SDFoundfile = false;                     //if opening the file "CTdata" was true=1=succesful false=0=insuccesful
int writeNote = false;                       //variable for when to write a note too SD

unsigned long SDTimer = 0;                     //variable for when to writeto SD (for millis function)
unsigned long writeIntervalSD = 60000;         //variable for time between writings to the SD (0 = infinite)------------------------------
unsigned  h, m;                             //variable for displaying the time between writings to the SD
unsigned long experimentDuration = 4294967295;   //variable for the duration of the experiment (4294967295 is the max value and is set as infi)
unsigned  hI, mI, sI;                            //intervalo ------------------------------------------------------------------


float raw_NO2 = 0;                      //variable to store the value comming from the analog pin
float NO2_Volt;                         //tnfnftnftnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
float NO2_R = 0;                        //NO2 Sensor Resistance
float NO2_ppb = 0;                      //ppb NO2 value

int raw_CO2 = 0;                         //variable to store the value coming from the analog pin
double CO2_Volt;                         //tnfnftnftnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
int voltage_diference;                    //tnfnftnftnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
float concentration;                     //global variable to store the CO2 concentration. Global because of the SDreader
int raw_CO = 0;                          //variable to store the value coming from the analog pin
double CO_ppm;                           //tnfnftnftnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn

#define LENG 31                          //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value = 0;                      //define PM1.0 value of the air detector module
int PM2_5Value = 0;                     //define PM2.5 value of the air detector module
int PM10Value = 0;                      //define PM10 value of the air detector module


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel


unsigned menuDisplay = 1;                           //oidcbçsebçvoisvoisnevoise

int setupMenu = 1;                       //variable for the setupMenus screen          - talves screen
int runSetup = 1;                        //variable for when the setup is occuring
bool runOnce = false;                     //variable for only letting screen text be written once (and prevent static)   ----
bool runOnce2 = false;








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

unsigned long debounceTime;
unsigned long setupDuration;
bool troubleshootingOn = false;
int cursorPlace = 1;
unsigned long  amountTime;




//////////////////////////////////////////////////////////////////////////////////////////////////void setup()////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {

  lcd.createChar(0, noteSD);   // ------------------------------------------------------

  pinMode(SDPin, OUTPUT);                //necessary for SDreader to work
  pinMode (buttonsPin, INPUT);           //define A0 (buttons's pin) as input

  Serial.begin(9600);                    //start the Serial comunication with the PM2.5
  Serial.setTimeout(1500);               //if it does not start ignore

  analogReference(DEFAULT);              //set the default voltage for reference voltage


  // Beginning of setup
  lcd.createChar(0, noteSD);
  lcd.begin(16, 2);                      //start the LCD library

  /*
    lcd.clear();                           //clean the display before write
    lcd.print("Ambient meter");            //message to show
    lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
    lcd.print("Wait5m-stabelize");         //message to show                                                        -------------------------
    delay(8000);                           //time to wait so you can read the message

    lcd.clear();                           //clean the display before write
    lcd.print("Setup experiment");         //message to show
    lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
    lcd.print("Wait5m-stabelize");         //message to show
    delay(5000);                           //time to wait so you can read the message

  */
  //.

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

        Serial.print("\nrunSetup-----");
        Serial.println(runSetup);
      }


      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          runSetup = runSetup + 2;
          runOnce = false;
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          runSetup ++;
          runOnce = false;
          experimentDuration = 3600000;
          break;
      }
    }
    Serial.print  ("   buttons");
    Serial.print  (buttonsSignal);


    ////////////////////////////////////////////////////////////////////////////menu-2


    if (runSetup == 2) {
      while (runOnce == false) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Timer setup:");
        runOnce = true;
        buttonsSignal = 0;

        Serial.print("  runSetup");
        Serial.print(runSetup);

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
      /*
            Serial.print("  amountTime");
            Serial.print(amountTime);
      */

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed

          cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          cursorPlace = cursorPlace + (1 - (cursorPlace >= 2));
          break;

        case 3:                                        //the down button is being pressed
          experimentDuration = experimentDuration - amountTime * ( 1 - ( experimentDuration <= 0));        //3  subtrack -----
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

        Serial.print("runSetup-----");
        Serial.println(runSetup);
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

        Serial.print("runSetup-----");
        Serial.println(runSetup);

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

      switch (buttonsSignal) {
        case 1:                                          //the most left button is being pressed
          cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          cursorPlace = cursorPlace + (1 - (cursorPlace >= 3));


          break;

        case 3:                                        //the down button is being pressed
          writeIntervalSD = writeIntervalSD - amountTime * ( 1 - ( writeIntervalSD <= 0));                 //3  subtrack more a---
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
    StationData = SD.open("CTdata.TXT", FILE_WRITE);
    SDFoundfile = true;                                   //means that the file "CTdata" was found; true=1=succesful false=0=insuccesful
    StationData.print("Beginning of experience - write frequency: ");
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

  Serial.print(menuDisplay);
  Serial.print("---");
  Serial.println(buttonsSignal);


  if (millis() > debounceTime + 100) {
    debounceTime = millis();
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

  Serial.print("  ExDuration");
  Serial.print(experimentDuration);
  Serial.print("  millis");
  Serial.print(millis());
  Serial.print(  "setupDuration");
  Serial.print(setupDuration);
  Serial.print(" resultado");
  Serial.print(experimentDuration >= (millis() - setupDuration ));

  if (experimentDuration >= (millis() - setupDuration )) {
    if (millis() - SDTimer > writeIntervalSD) {
      SDTimer = millis();      //load the millis time to the SDTimer
      runOnce = false;

      raw_NO2 = analogRead(NO2_In);          //read the analog in value
      NO2_Volt = raw_NO2 / 409.2;     //conversion formula
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
      StationData = SD.open("CTdata.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        StationData.println("-----------------------------");        //note that'll be written to SD file
        writeNote = false;                                                  //reset node warning (note has been written)
        runOnce = false;

      }
      StationData.println(concentration);                                   //SD write CO2 concentration
      StationData.println(CO_ppm);
      StationData.println(PM01Value);
      StationData.println(PM2_5Value);
      StationData.println(PM10Value);
      StationData.println(NO2_ppb);

      fileSize = StationData.size();                               //read file size
      Serial.println(StationData.size()); ///help for troubleshooting
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
      if ((CO2_Volt < 400) & CO2_Volt != 0 ) {               //////////////////////thisssss
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




    if (((menuDisplay == 4) | writeNote) & (runOnce == false))  {
      runOnce = true;
      lcd.clear();                                    //clean the display before write
      lcd.setCursor(0, 0);                            //set the LCD cursor position first column(0) and second line(0)
      if (SD.begin() | troubleshootingOn) {                 //check if it has permition to write and if SD is connected

        lcd.print("SavingSD ");                       //message to show



        if (experimentDuration != 4294967295) {
          lcd.setCursor(9, 0);
          int hTimer = ((experimentDuration - millis() + setupDuration) / 3600000);
          int mTimer = ((experimentDuration - millis() + setupDuration) - (3600000 * hI)) / 60000;
          lcd.print(hTimer);                       //message to show
          lcd.print("h");                       //message to show
          lcd.print(mTimer);                       //message to show
          lcd.print("m");                       //message to show

          Serial.print("mTimer");
          Serial.print(mTimer);

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
    }
  }

  else {


    while (runOnce2 == false)
    {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print ("Over.  Continue?");
      lcd.setCursor(0, 1);
      lcd.print ("1-Yes   2-No");
      runOnce2 = true;
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             

    switch (buttonsSignal) {
      case 1:                                         //no buttons is being pressed (divider can should be changed if button reading is wrong)
        experimentDuration = 4294967295;
        SD.open("CTdata.TXT", FILE_WRITE);
        StationData.println("Experiment continued");
        StationData.close();
        break;
      case 2:                                         //the most left button is being pressed
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print ("Done!!");
        SD.open("CTdata.TXT", FILE_WRITE);
        StationData.println("End of experiment");
        StationData.close();
        while (true) {

        }
        break;

    }
  }

  if (Serial & troubleshootingOn) {
    /* Serial.print("SDWriting-"); ///help for troubleshooting
      Serial.println(SDWriting); ///help for troubleshooting
      Serial.print("SDFoundfile-"); ///help for troubleshooting
      Serial.println(SDFoundfile); ///help for troubleshooting
      Serial.print("writeIntervalSD-"); ///help for troubleshooting
      Serial.println(writeIntervalSD); ///help for troubleshooting
      Serial.print("timeModeLCD"); ///help for troubleshooting
      Serial.println(timeModeLCD); ///help for troubleshooting
      Serial.print("button "); ///help for troubleshooting
      Serial.println(buttonsSignal ); ///help for troubleshooting
      //Serial.print("writeNote "); ///help for troubleshooting
      //Serial.println(writeNote ); ///help for troubleshooting
      //Serial.print("SD.begin "); ///help for troubleshooting
      //Serial.println(SD.begin() ); ///help for troubleshooting
      Serial.println(" "); ///help for troubleshooting*/
  }
  Serial.print(" Duration");
  Serial.println(experimentDuration);

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
