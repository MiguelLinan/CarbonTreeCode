/*******************************************************
  Medidor ambiente
  modificada 9.0
  2021/06/07

  Author:
  Miguel Nunes Alves liñan da Silva

  Based on:
  "Name and authors of previous version"
*******************************************************/

//In the arduino code time is measured in milliseconds (ms)

#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>



// Pin mapping for the display
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel

byte warning[] = {    //warning "!!" character                 *º*º*º*º verificar se está bem
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

byte selected[] = {
  B00000,
  B00001,
  B00011,
  B10110,
  B11100,
  B01000,
  B00000,
  B00000
};

byte notSelected[] = {
  B00000,
  B10001,
  B01010,
  B00100,
  B01010,
  B10001,
  B00000,
  B00000
};

byte arrowDown[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B11111,
  B01110,
  B00100,
  B00000
};

byte arrowUp[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B00100,
  B00100,
  B00100,
  B00100
};



//quick access variables:
unsigned long writeIntervalSD = 60000;         //variable for time between writings to the SD (Default)
unsigned long experimentDuration = 3600000;   //variable for the duration of the experiment (Default)(4294967295 is the max value and is set as infinite)


//some of the pins: /pinout
const byte SDPin = 2;                              //SDreader pin
const byte NO2_In = A1;                            //NO2 sensor pin
const byte CO_In = A2;                          //CO sensor pin
const byte CO2_In = A3;                         //CO2 sensor pin
#define SPI_SS = SDPin;                     //define the default SS(slave select) as pin 2

//variables for the buttons:
const byte buttonsPin = A0;                        //pin of the LCD buttons
//unsigned long debounceMillis;               //minimal amount of time in between button presses

#define LENG 31                          //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];







/* Start from scratch _Boy_

                   USE FuNctIonTs Like in PyThonnnnnnnn

    New features:
    - Choose sensor to use   (@^@)
    - See list of files in the SD (at least in the beggining)    (@^@)--------------------------------------------------------------------------------------------------
    - Set the name of the file ( as a number from 1 to howmuch as I can, create a system that warns if that name is taken)    (@^@)-------------------------------------
    - Set the start time ( if this feature is addded the numbers feature might be useless)    (@^@)---------------------------------------------------------------------
    - Store sensors value in diferent files (files will have the name = (number) + (sensor) + (Start time) ex 23_CO_12:30 // - csv = better ?data on name/hour? possible
    - Better running time stats and menus:
    - See all files being written and their size (manualy choosing which to see + seeing their name(perhaps)) //no
    - Arrows indicating if i can move up or down(pixel art) kinda
    -

    - Guardar ficheiros em CSV (comma separeted values) com o valor de cada cena em coluna e mais uma para a nota de se abriu ou fechou porta(diferentes sinais) (@^@)--



*/


///ex

const char  listSensors[][4]  = {"CO2", "NO2", "CO ", "PMs"};  // is PROGMEM worth it ?º*º*º*º*º*º*º*ª*

bool activeSensors[] = {false, false, false, false};

char fileName[13];


unsigned long lastTime;
const unsigned int refreshRate = 150;  //refresh rate for buttons

unsigned long setupDuration = 0;
unsigned long endlessExit = 0;    //contar o tempo para sair do endless
const unsigned int endlessExitTime = 6000;  //refresh rate for buttons

byte line = 0;
byte iSensor = 0;


unsigned  int h, m;                       //variable for displaying the duration of the experiment to the LCD





File dataFile;

void setup() {


  Serial.begin(9600);
  lcd.begin(16, 2);

  lcd.createChar(0, noteSD);   
  lcd.createChar(1, warning);
  lcd.createChar(2, arrowUp);
  lcd.createChar(3, arrowDown);
  lcd.createChar(4, selected);
  lcd.createChar(5, notSelected);

  if (true) {
    Serial.begin(9600);
  }

  if (getButtons() != 5) {
    welcome();
  }

  while (true) {


    lastTime = 0;
    setupDuration = 0;
    writeIntervalSD = 60000;
    experimentDuration = 3600000;


    timer ();
    delay(500);
    interval ();
    delay(500);



    sensors();
    // delay(700);
    while (true) {
      delay(150);
      if (SD.begin(2)) {
        files();
        break;
      }
      else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("SD not connected"));
        lcd.setCursor(0, 1);
        lcd.print(F("1-Skip"));
        if (getButtons() == 1) {
          break;
        }
      }
    }

    setupDuration = millis();
    if (!mainScreen()) {           // if main is false: end
      lcd.clear();                   //clean the display before write
      lcd.setCursor(4, 0);                  
      lcd.print(F("SESSION"));
      lcd.setCursor(1, 1); 
      lcd.write(byte(4)); 
      lcd.print(F(" COMPLETED ")); 
      lcd.write(byte(4)); 
      return;
    }




  }

}

void loop() {
}


//Subrotines /spelling ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void welcome() {                                                        // Improveº*º*º*º*º*º*º*ª*º*º*º*º*º*º*º*ª*
  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("**Carbon Tree**"));                   //message to show
  lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
  lcd.print(F(" Portugal 2022"));                   //message to show
  delay(4000);                   //wait 4s
  if (getButtons() == 5) {
    return;
  }
  lcd.clear();                           //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F(" Ambient meter"));                    //message to show
  delay(3000);                    //wait 3s
  if (getButtons() == 5) {
    return;
  }
  lcd.clear();                           //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.write(byte(1));                    //display the warning character
  lcd.print(F("Wait 5 min"));                   //message to show
  lcd.write(byte(1));                    //display the warning character
  lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
  lcd.print(F("to stabilize"));                   //message to show
  delay(4000);                   //wait 4s
}

//--//--//--//--//--//--//--//--//--//--//--//--//TIMER

void timer () {

  byte cursorPlace = 1;
  int lenghtH;                   //for counting the number of characters
  int lenghtM;                   //for counting the number of characters
  unsigned long  amountTime;               //variable for how much time to add

  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("Set timer:"));                   //message to show


  lcd.setCursor(0, 1);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("1-Endless 2-Set"));                   //message to show

  delay(700);

  while (getButtons() != 2) {
    if (getButtons() == 1) {
      experimentDuration = 4294967295;                 //4294967295 is the maximum value and it is our "infinite" time
      return;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Timer setup:"));
  lcd.blink();     //makes the LCD start blinking
  while (true) {
    if (millis() - lastTime > refreshRate) {
      lastTime = millis();
      lcd.setCursor(1, 1);
      h = (experimentDuration / 3600000);                     //converts milliseconds to hours
      m = (experimentDuration - (3600000 * h)) / 60000;       //converts milliseconds to minutes
      lcd.print(h);                             //shows the time in hours
      lcd.print(F("h:"));
      lcd.print(m);                             //shows the time in minutes
      lcd.print(F("m     "));

      lenghtH = log10(h);                   //counts the number of characters - 1
      lenghtM = log10(m);                   //counts the number of characters - 1

      switch (cursorPlace) {
        case 1:         //cursor in h
          lcd.setCursor(lenghtH + 1, 1);          //set cursor to h positions
          amountTime = 3600000;                   //makes the time change in hours
          break;
        case 2:         //cursor in m
          lcd.setCursor(lenghtH + 4 + lenghtM , 1);          //set cursor to m positions
          amountTime = 60000;                                //makes the time change in minutes
          break;
      }


      if (experimentDuration < 60000) {              //minimal limit of 1m
        experimentDuration = 60000;
      }

      switch (getButtons()) {
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
          lcd.noBlink();                     //makes the LCD stop blinking
          return;
      }
    }
  }
}
//--//--//--//--//--//--//--//--//--//--//--//--//INTERVAL
void interval () {

  byte cursorPlace = 1;
  int lenghtHI, lenghtMI, lenghtSI;                   //for counting the number of characters
  unsigned long  amountTime;               //variable for how much time to add
  unsigned  int hI, mI, sI;                    //variable for displaying the time between writings to the SD and LCD
  byte button, lastButton;


  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("SetSD frequency:"));                   //message to show


  lcd.setCursor(0, 1);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("1-Default 2-Set"));                   //message to show

  delay(700);

  while (getButtons() != 2) {
    if (getButtons() == 1) {
      return;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Save interval:"));
  lcd.blink();     //makes the LCD start blinking
  while (true) {
    if (millis() - lastTime > refreshRate) {
      lastTime = millis();
      lcd.setCursor(1, 1);
      hI = (writeIntervalSD / 3600000);                     //converts milliseconds to hours
      mI = (writeIntervalSD - (3600000 * hI)) / 60000;       //converts milliseconds to minutes
      sI = ((writeIntervalSD - (3600000 * hI)) - (mI * 60000)) / 1000;  //converts milliseconds to seconds
      lcd.print(hI);                             //shows the time in hours
      lcd.print(F("h:"));
      lcd.print(mI);                             //shows the time in minutes
      lcd.print(F("m:"));
      lcd.print(sI);                             //shows the time in minutes
      lcd.print(F("s    "));

      lenghtHI = log10(hI);                   //counts the number of characters - 1
      lenghtMI = log10(mI);                   //counts the number of characters - 1
      lenghtSI = log10(sI);                  //counts the number of characters - 1



      switch (cursorPlace) {
        case 1:         //cursor in h
          lcd.setCursor(lenghtHI + 1, 1);          //set cursor to h positions
          amountTime = 3600000;                   //makes the time change in hours
          break;
        case 2:         //cursor in m
          lcd.setCursor(lenghtHI + 4 + lenghtMI , 1);          //set cursor to m positions
          amountTime = 60000;                                //makes the time change in minutes
          break;
        case 3:            //cursor in s
          lcd.setCursor(lenghtHI + lenghtMI + lenghtSI + 7, 3);                     //set cursor to s positions
          amountTime = 1000;
          break;
      }


      if (writeIntervalSD < 10000) {              //minimal limit of 1m
        writeIntervalSD = 10000;
      }


      button = getButtons();
      switch (button) {
        case 1:                                          //the most left button is being pressed
          if (button != lastButton) {
            cursorPlace = cursorPlace - (1 - (cursorPlace <= 1));     //moves cursor to the left
          }
          break;

        case 2:                                         //the  second button couting from the left is being pressed
          if (button != lastButton) {
            cursorPlace = cursorPlace + (1 - (cursorPlace >= 3));     //moves cursor to the right
          }
          break;

        case 3:                                        //the down button is being pressed
          if (writeIntervalSD >= amountTime) {            //prevents variavle rollover
            writeIntervalSD = writeIntervalSD - amountTime;         //reduces the experiment duration
          }
          break;

        case 4:                                         //the up button is being pressed
          writeIntervalSD = writeIntervalSD + amountTime;    //increases the experiment duration
          break;

        case 5:                                          //the right button is being pressed
          lcd.noBlink();                     //makes the LCD stop blinking
          return;
      }
      lastButton = button;
    }
  }
}
//--//--//--//--//--//--//--//--//--//--//--//--//SENSORS

void sensors() {
  int i = 0;
  int lastButton;
  int button;


  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("Pick Sensors:"));                   //message to show


  lcd.setCursor(0, 1);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("1-All 2-Choose"));                   //message to show

  while (getButtons() != 2) {
    if (getButtons() == 1) {
      for (int i = 0; i < sizeof activeSensors; i++) {
        activeSensors[i] = true;  //loop through all activeSensors and make them all true (activate all sensors)
      }
      return;
    }
  }

  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("Pick Sensors:"));                   //message to show


  lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
  lcd.print(listSensors[i]);                   //message to show

  while (true) {
    if (millis() - lastTime > refreshRate) {
      lastTime = millis();
      button = getButtons();
      if (button != lastButton) {
        lastButton = button;

        switch (button) {
          case 0:
            break;
          case 1:
            activeSensors[i] = !activeSensors[i];
            break;
          case 2:
            break;
          case 3:
            i += 1;
            if (i > 3) {
              i = 3;
            }
            break;
          case 4:
            i -= 1;
            if (i < 0) {
              i = 0;
            }
            break;
          case 5:
            if (activeSensors[0] | activeSensors[1] | activeSensors[2] | activeSensors[3]) {
              return;
            }
        }

        lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
        lcd.print(listSensors[i]);                   //message to show

        lcd.setCursor(12, 1);
        if (activeSensors[i]) {
          lcd.write(byte(4));
        }
        else {
          lcd.write(byte(5));
        }


        lcd.setCursor(15, 0);                   //set the LCD cursor position first column(0) and second line(1)
        if (i != 0) {
          lcd.write(byte(2));                   //message to show
        }
        else {
          lcd.print(F(" "));                   //message to show
        }
        lcd.setCursor(15, 1);                   //set the LCD cursor position first column(0) and second line(1)
        if (i != 3) {
          lcd.write(byte(3));                   //message to show
        }
        else {
          lcd.print(F(" "));                   //message to show
        }
      }

    }
  }
}

//--//--//--//--//--//--//--//--//--//--//--//--//FILES

void files() {
  int i = 0;
  int lastButton;
  int button = 0;


  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("Save file:"));                   //message to show


  lcd.setCursor(0, 1);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("1-New 2-On old"));                   //message to show

  delay(700);

  while (getButtons() != 2) {
    if (getButtons() == 1) {

      ////////CODE//open new file
      openFile(false);
      return;
    }
  }

  lcd.clear();                   //clean the display before write
  lcd.setCursor(0, 0);                    //set the LCD cursor position first column(0) and first line(0)
  lcd.print(F("Choose file:"));                   //message to show

  lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)




  //get names and put them in char
  const int numberFiles =  13;
  const int fileNameLength = 13;
  char fileNames[numberFiles][fileNameLength];
  int filesExist = 0;   //know how many files in SD card for UI experience
  File root = SD.open("/");
  File entry =  root.openNextFile();
  entry =  root.openNextFile();

  int index = 0;
  while (entry) {
    strncpy(fileNames[index], entry.name(), fileNameLength);
    index++;
    filesExist++;
    entry =  root.openNextFile();
  }
  //


  while (true) {
    if (millis() - lastTime > refreshRate) {
      lastTime = millis();
      button = getButtons();
      if (button != lastButton) {
        lastButton = button;

        switch (button) {
          case 0:
            break;
          case 1:    // voltar a trás ?


            openFile(false);  /////////tirar a seguir

            return;
          case 2:    // próximo ?
            break;
          case 3:  //
            i += 1;
            if (i > filesExist - 1) {
              i = filesExist - 1;
            }
            break;
          case 4: //
            i -= 1;
            if (i < 0) {
              i = 0;
            }
            break;
          case 5:
            strcpy(fileName, fileNames[i]);
            openFile(true);
            return;
            //activeSensors[i] = !activeSensors[i];  - make open and apend
        }


        lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
        lcd.print(fileNames[i]);                   //message to show

        lcd.setCursor(13, 1);
        lcd.print(F("< "));                    //melhor setadsfuhslyufge

        lcd.setCursor(15, 0);                   //set the LCD cursor position first column(0) and second line(1)
        if (i != 0) {
          lcd.write(byte(2));                   //message to show
        }
        else {
          lcd.print(F(" "));                   //message to show
        }
        lcd.setCursor(15, 1);                   //set the LCD cursor position first column(0) and second line(1)
        if (i != filesExist - 1) {
          lcd.write(byte(3));                   //message to show
        }
        else {
          lcd.print(F(" "));                   //message to show
        }
      }
    }
  }
}

//--//--//--//--//--//--//--//--//--//--//--//--//OPEN FILE

void openFile (bool append) {

  if (append) {
    dataFile = SD.open(fileName, FILE_WRITE);
  }
  else {
    subOpenFile();
  }

  if (dataFile) {
    for (int i = 0; i < sizeof activeSensors; i++) {
      if (activeSensors[i]) {
        dataFile.print(listSensors[i]);
        dataFile.print(";");
      }
    }

    dataFile.print(writeIntervalSD / 1000); //make it seconds not ms
    dataFile.print(";");
    dataFile.println(experimentDuration / 1000); //make it seconds not ms
  }
  dataFile.close();
}

void subOpenFile() {
  char text[5] = "data";
  strcpy(fileName, text);
  for (int n = 0;  n < 10; n++) {
    for (int nn = 0;  nn < 10; nn++) {
      char temp[3] = {n + 48, nn + 48, '\0'};
      fileName[sizeof(text) - 1] = '\0';
      strcat(fileName, temp);
      Serial.print("2-"); Serial.print(n); Serial.print("-->"); Serial.println(fileName);
      strcat(fileName, ".csv");
      Serial.print("3-"); Serial.print(n); Serial.print("-->");; Serial.println(fileName);

      if (!SD.exists(fileName)) {
        dataFile = SD.open(fileName, FILE_WRITE);
        Serial.print("free! -");
        Serial.println(fileName);
        return;
      }
    }
  }
}



//--//--//--//--//--//--//--//--//--//--//--//--//MAINSCREEN


bool mainScreen() {

  float sensorData [7] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};


  bool writeNote = false;
  unsigned long SDTimer;
  bool statusSD;
  byte button, lastButton;
  bool refreshScreen = true;
  unsigned long fileSize = 0;




  byte menuDisplay = 0;
  byte  numberDisplays = 1;   // the 1 is for the calculations
  for (byte i = 0; i < 4; i++) { //count how many displays are needed
    if (activeSensors[i]) {
      numberDisplays++;
      if (i == 3) {
        numberDisplays += 2;
      }
    }
  }
  numberDisplays = numberDisplays / 2;             // 2 is the number of sensors possible to show at the same time
  Serial.print(F("-numberDDDisplays->")); Serial.print(numberDisplays);



  while (millis() - setupDuration < experimentDuration) {
    if (millis() - SDTimer > writeIntervalSD) {
      SDTimer = millis();
      refreshScreen = true;

      //get data
      if (activeSensors[0]) { //CO2
        int raw_CO2 = analogRead(CO2_In);          //read the analog in value
        double CO2_Volt = raw_CO2 * (5000 / 1024.0); //conversion formula
        int voltage_diference = CO2_Volt - 400;
        sensorData[0] = voltage_diference * (50.0 / 16.0); //auxiliar calculation
      }
      if (activeSensors[1]) { //NO2
        float raw_NO2 = analogRead(NO2_In);          //read the analog in value
        float NO2_Volt = raw_NO2 / 409.2;     //conversion formula
        float NO2_R = 22000 / ((5 / NO2_Volt) - 1);  //find sensor resistance from No2_Volt, using 5V input & 22kOhm load resistor
        sensorData[1] = (.000008 * NO2_R - .0194) * 1000; //convert Rs to ppb (parts per billion) concentration of NO2
      }
      if (activeSensors[2]) { //CO
        int raw_CO = analogRead(CO_In);          //read the analog in value
        sensorData[2] = 3.027 * exp(1.0698 * (raw_CO * (5.0 / 1024))); //conversion formula
      }
      if (activeSensors[3]) { //PMs
        if (Serial.find(0x42)) {            //look for the 0x42 in the Serial port
          Serial.readBytes(buf, LENG);      //load the buf with the values
          if (buf[0] == 0x4d) {
            if (checkValue(buf, LENG)) {
              sensorData[3] = transmitPM01(buf);   //count PM1.0 value of the air detector module
              sensorData[4] = transmitPM2_5(buf);  //count PM2.5 value of the air detector module
              sensorData[5] = transmitPM10(buf);   //count PM10 value of the air detector module
            }
          }
        }
        else {
          sensorData[3] = -1;   //error number
          sensorData[4] = -1;  //error number
          sensorData[5] = -1;   //error number
        }
      }
      //save data
      dataFile = SD.open(fileName, FILE_WRITE);
      if (dataFile) {
        for (byte i; i < (sizeof(activeSensors)); i++) {
          if (activeSensors[i]) {
            dataFile.print(sensorData[i]); dataFile.print(F(";"));
            if (activeSensors[i] == (sizeof(activeSensors) - 1)) {
              dataFile.print(sensorData[i + 1]); dataFile.print(F(";"));
              dataFile.print(sensorData[i + 2]); dataFile.print(F(";"));
            }
          }
        }
        //save note
        if (writeNote) {
          dataFile.print(F("--Note--;"));
          writeNote = false;
        }
        dataFile.println();
        fileSize = dataFile.size();
        dataFile.close();
        statusSD = true;
      }
      else {
        statusSD = false;
      }
    }

    //buttons and display
    if (millis() - lastTime > refreshRate) {
      lastTime = millis();
      button = getButtons();
      if (button != lastButton) {
        lastButton = button;

        switch (button) {
          case 0:                                         //no buttons is being pressed (divider can should be changed if button reading is wrong)
            break;
          case 1:                                         //the most left button is being pressed
            break;

          case 2:                                         //the  second button couting from the left is being pressed
            break;

          case 3:                                         //the down button is being pressed
            menuDisplay =  menuDisplay + ( 1 - ( menuDisplay >= numberDisplays));   //scroll down

            refreshScreen = true;
            break;

          case 4:                                         //the up button is being pressed
            menuDisplay = menuDisplay - ( 1 - ( menuDisplay <= 0 ));   //scroll up
            refreshScreen = true;
            break;

          case 5:                                         //the right button is being pressed
            writeNote = true;                             //write note to "CTdata.TXT" file
            refreshScreen = true;
            break;
        }
      }
      else if (button == 1) {
        if (millis() - endlessExit > endlessExitTime) {
          return false;
        }
      }
      else {
        endlessExit = millis();
      }


      byte  shownDisplays = 0;
      line = 0;
      int  order = 0;


      Serial.print(F(" -DisplayP->")); Serial.print(menuDisplay);

      if (refreshScreen) {
        refreshScreen = false;
        lcd.clear();
        if (menuDisplay != numberDisplays) {
          for (iSensor = 0; iSensor < (sizeof(activeSensors) + 2); iSensor++) {  //(sizeof(activeSensors) drgdrg
            Serial.print("--For--iSensor-->"); Serial.print(iSensor);
            if (shownDisplays == 2) {
              Serial.print("--enough-->");
              break;
            }

            if (activeSensors[iSensor] || ((iSensor > 3) & activeSensors[3])) {
              //Serial.print("--half-->");

              order ++;


              if (order > ((menuDisplay * 2) - 0)) {
                Serial.print(F("--run!-->")); Serial.print(iSensor);
                lcd.setCursor(0, line);
                displayData(sensorData[iSensor]);
                line++;
                shownDisplays ++;

              }
            }
            Serial.print(" ");
          }
        }
        else {
          lcd.setCursor(0, 0);
          if (statusSD) {                 //only runs if SD is connected
            if (experimentDuration != 4294967295) {               //only runs if experiment duration is not infinite
              lcd.print(F("Ends in: "));                       //message to show
              lcd.setCursor(9, 0);
              int hTimer = ((experimentDuration - millis() + setupDuration) / 3600000);                       //converts milliseconds to hours
              int mTimer = ((experimentDuration - millis() + setupDuration) - (3600000 * hTimer)) / 60000;    //converts milliseconds to minutes
              lcd.print(hTimer);                       //message to show
              lcd.print(F("h"));                       //message to show
              lcd.print(mTimer);                       //message to show
              lcd.print(F("m"));                       //message to show
            }
            else {
              lcd.print(F("Endless mode"));                       //message to show
            }
            if (writeNote) {
              lcd.setCursor(15, 0);                       //set the LCD cursor position column 15(16) and first line(0)
              lcd.write(byte(0));                         //indicates that it needs to write a note("---------------"), shows icon
            }

            lcd.setCursor(0, 1);                           //set the LCD cursor position first column(0) and second line(1)
            lcd.print(F("filesize:"));                        //message to show
            lcd.print(fileSize);                            //print file's size in bytes to the lcd
          }
          else {
            lcd.print(F("SD not connected"));
          }



        }
      }
      Serial.println();





    }


  }

  lcd.clear();
  lcd.setCursor(1, 0);                           //set the LCD cursor position first column(1) and second line(0)
  lcd.print(F("time is over!"));                        //message to show
  lcd.setCursor(0, 1);                           //set the LCD cursor position first column(0) and second line(1)
  lcd.print(F("1-repeat  2-end"));                        //message to show
  while (true) {
    delay(100);
    if (getButtons() == 1) {
      return true;
    }
    else if (getButtons() != 0) {
      return false;
    }

  }
}


void displayData(float sensorFloat) {

  // Serial.print(" -sensor->"); Serial.print(sensor); Serial.print("<-->");

  if (iSensor == 0) {
    displayCO2(sensorFloat);
  }
  if (iSensor == 1) {
    displayNO2(sensorFloat);
  }
  if (iSensor == 2) {
    displayCO(sensorFloat);
  }
  if (iSensor == 3) {
    displayPM1_0(sensorFloat);
  }
  if (iSensor == 4) {
    displayPM2_5(sensorFloat);
  }
  if (iSensor == 5) {
    displayPM_10(sensorFloat);
  }
}


void displayCO2(float sensorFloat) {
  lcd.print(F("CO2: "));
  if (sensorFloat > 0)  {
    lcd.print(sensorFloat); lcd.setCursor(13, line); lcd.print(F("ppm"));
  }
  else {
    lcd.print(F("error"));
  }
  Serial.print("--co2-->"); Serial.print(sensorFloat);
}

void displayNO2(float sensorFloat) {
  lcd.print(F("NO2: "));
  if (sensorFloat > 0) {
    lcd.print(sensorFloat); lcd.setCursor(13, line); lcd.print(F("ppb"));
  }
  else {
    lcd.print(F("error"));
  }
}

void displayCO(float sensorFloat) {
  lcd.print(F("CO: "));
  if (sensorFloat > 0) {
    lcd.print(sensorFloat); lcd.setCursor(13, line); lcd.print(F("ppm"));
  }
  else {
    lcd.print(F("error"));
  }
}

void displayPM1_0(float sensorFloat) {
  lcd.print(F("PM1.0: "));
  if ((sensorFloat >= 0) & (sensorFloat < 4000))  {  //make overflow ovf an error
    lcd.print(sensorFloat);  lcd.setCursor(11, line); lcd.print(F("ug/m3"));                                             // lcd.setCursor(); make it
  }
  else {
    lcd.print(F("error"));
  }

}

void displayPM2_5(float sensorFloat) {
  lcd.print(F("PM2.5: "));
  if (sensorFloat >= 0) {
    lcd.print(sensorFloat); lcd.setCursor(11, line); lcd.print(F("ug/m3"));
  }
  else {
    lcd.print(F("error"));
  }
}

void displayPM_10(float sensorFloat) {
  lcd.print(F("PM10: "));
  if (sensorFloat >= 0) {
    lcd.print(sensorFloat); lcd.setCursor(11, line); lcd.print(F("ug/m3"));
  }
  else {
    lcd.print(F("error"));
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int getButtons() {
  int buttonsValue = analogRead(buttonsPin);     //output from the LCD buttons (divisor pode ser alterado se a leitura dos botões estiver com descrepãncias)
  if (buttonsValue < 190) {
    return  5;
  } else if (buttonsValue < 370) {
    return  4;
  } else if (buttonsValue < 510) {
    return  3;
  } else if (buttonsValue < 720) {
    return  2;
  } else if (buttonsValue < 900) {
    return  1;
  } else {
    return 0;
  }
}

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


/*Problemas:




  file info menu

  make sure there is no sensor is not an option - if all false make all true

  show when maximum of files is open
  sim não / setas cima baixo/



*/
