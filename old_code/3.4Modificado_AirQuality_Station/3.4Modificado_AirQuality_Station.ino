/*******************************************************
  Medidor ambiente
  modificada 3.4
  2021/01/28
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


File CO2datafile;
unsigned long fileSize;                    //size in bytes of the SD file
int SDFoundfile = false;                     //if opening the file "CO2data" was true=1=succesful false=0=insuccesful
int SDWriting = true;                        //permission for SD do write true=1=write false=0=nothing
int writeNote = false;                       //variable for when to write a note too SD


long intervalMode1 = 5000;                      //variable for time inbetween writes to the SD (ms)                                                      (~~)
long intervalMode2 = 60000;                     //variable for time inbetween writes to the SD (ms)                                                      (~~)
int timeModeLCD = 2;                           //variable for LCD to know the time between writings to the SD (set to the intervalMode2 by default)
long writeInterval = intervalMode2;   //variable for time between writings to the SD default (set to the intervalMode2 by default)
unsigned long SDTimer = 0;                     //variable for when to writte to SD


float raw_NO2 = 0;                      //variable to store the value comming from the analog pin
float NO2_R = 0;                        //NO2 Sensor Resistance
float NO2_ppb = 0;                      //ppb NO2 value

int raw_CO2 = 0;                         //variable to store the value coming from the analog pin
float concentration;                     //global variable to store the CO2 concentration. Global because of the SDreader
int raw_CO = 0;                          //variable to store the value coming from the analog pin

#define LENG 31                          //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value = 0;                      //define PM1.0 value of the air detector module
int PM2_5Value = 0;                     //define PM2.5 value of the air detector module
int PM10Value = 0;                      //define PM10 value of the air detector module


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel

unsigned long ecra1Timer = 0;            //variable for the time ON first screen
unsigned long ecra2Timer = 0;            //variable for the time ON second screen
unsigned long ecra3Timer = 0;            //variable for the time ON third screen
unsigned long ecra4Timer = 0;
unsigned long intervalo = 5000;          //variable for time exhibition between screens


bool ecra1 = true;                       //variable to force first screen
bool ecra2 = false;                      //variable to force second screen
bool ecra3 = false;                      //variable to force third screen
bool ecra4 = false;                      //variable to force fourth screen

void setup() {

  pinMode(SDPin, OUTPUT);                //necessary for SDreader to work
  pinMode (buttonsPin, INPUT);           //define A0 as input


  lcd.begin(16, 2);                      //start the LCD library
  lcd.clear();                           //clean the display before write
  lcd.print("Ambient meter");        //message to show
  lcd.setCursor(0, 1);                   //set the LCD cursor position first column(0) and second line(1)
  lcd.print("Wait5m-stabelize");        //message to show
  delay(intervalo);                      //time to wait so you can read the message

  analogReference(DEFAULT);              //set the default voltage for reference voltage

  Serial.begin(9600);                    //start the Serial comunication with the PM2.5
  Serial.setTimeout(1500);               //if it does not start ignore

  if (SD.begin()) {                                      //checking if SDreader and SDcard are connected and start the SD library
    CO2datafile = SD.open("CO2data.TXT", FILE_WRITE);
    SDFoundfile = true;                                   //means that the file "CO2data" was found; true=1=succesful false=0=insuccesful
    fileSize = CO2datafile.size();
    CO2datafile.close();

  }
}

void loop() {

  raw_NO2 = analogRead(NO2_In);          //read the analog in value
  float NO2_Volt = raw_NO2 / 409.2;     //conversion formula
  NO2_R = 22000 / ((5 / NO2_Volt) - 1);  //find sensor resistance from No2_Volt, using 5V input & 22kOhm load resistor
  NO2_ppb = (.000008 * NO2_R - .0194) * 1000; //convert Rs to ppb (parts per billion) concentration of NO2

  raw_CO2 = analogRead(CO2_In);          //read the analog in value
  double CO2_Volt = raw_CO2 * (5000 / 1024.0); //conversion formula

  raw_CO = analogRead(CO_In);          //read the analog in value
  double CO_ppm = 3.027 * exp(1.0698 * (raw_CO * (5.0 / 1024))); //conversion formula

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

  if ((millis() - ecra1Timer > intervalo) && ecra1) {
    ecra2Timer = millis();                          //load the millis time to the ecra2Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = false;
    ecra2 = true;
    ecra3 = false;
    ecra4 = false;
    if (CO2_Volt == 0) {
      // print the results to the lcd
      lcd.print("CO2: ");                             //message to show
      lcd.print("Fault");                             //message to show
    }
    if (CO2_Volt < 400) {
      // print the results to the lcd
      lcd.print("CO2: ");                             //message to show
      lcd.print("preheating");                        //message to show
    }
    else {
      int voltage_diference = CO2_Volt - 400;
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

  if ((millis() - ecra2Timer > intervalo) && ecra2) {
    ecra3Timer = millis();                          //load the millis time to the ecra3Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = false;
    ecra2 = false;
    ecra3 = true;
    ecra4 = false;
    lcd.print("PM1.0: ");                           //message to show
    lcd.print(PM01Value);                           //print the results to the lcd
    lcd.print(" ug/m3");                             //message to show

    lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
    lcd.print("PM2.5: ");                           //message to show
    lcd.print(PM2_5Value);                          //print the results to the lcd
    lcd.print(" ug/m3");                             //message to show
  }

  if ((millis() - ecra3Timer > intervalo) && ecra3) {
    ecra4Timer = millis();                          //load the millis time to the ecra1Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = false;
    ecra2 = false;
    ecra3 = false;
    ecra4 = true;
    lcd.print("PM10: ");                            //message to show
    lcd.print(PM10Value);                           //print the results to the lcd
    lcd.print(" ug/m3");                            //message to show

    lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
    lcd.print("NO2: ");                             //message to show
    lcd.print(NO2_ppb);                             //print the results to the lcd
    lcd.print(" ppb");                              //message to show
  }




  if ((millis() - ecra4Timer > intervalo) && ecra4) {
    ecra1Timer = millis();                          //load the millis time to the ecra4Timer variable
    lcd.clear();                                    //clean the display before write
    lcd.setCursor(0, 0);                            //set the LCD cursor position first column(0) and second line(0)
    ecra1 = true;
    ecra2 = false;
    ecra3 = false;
    ecra4 = false;


    if (SDWriting && SD.begin()) {                  //check if it has permition to write and if SD is connected

      lcd.print("SDwriting-");                       //message to show
      switch (timeModeLCD) {                            //verify the SD reading interval, there can be added more predefenitions (if there are more buttons)
        //or the predefinitions can be changed by changing the values intervalMode1 and intervalMode2 (~~)
        case 1:
          lcd.print("5s");                           //message to show                                                                                        (~~)
          break;

        case 2:
          lcd.print("1m");                           //message to show                                                                                         (~~)
          break;
      }

      if (writeNote) {
        lcd.print("-Not");                           //indicates that it needs to writte a note("---------------")
      }

      lcd.setCursor(0, 1);                           //set the LCD cursor position first column(0) and second line(1)
      lcd.print("filesize:");                        //message to show
      lcd.print(fileSize);                            //print file's size in bytes to the lcd
    }

    else {
      lcd.print("SD not connected");                 //message to show
    }
  }




  buttonsValue = analogRead(buttonsPin);     //output from the LCD buttons (divisor pode ser alterado se a leitura dos botões estiver com descrepãncias)


  if (buttonsValue > 190) {                //interpreting the analog buttons signal
    buttonsSignal = 1;
    if (buttonsValue > 300) {
      buttonsSignal = 2;
      if (buttonsValue > 500) {
        buttonsSignal = 3;
        if (buttonsValue > 720) {
          buttonsSignal = 4;
          if (buttonsValue > 900) {
            buttonsSignal = 5;
          }
        }
      }
    }
  }
  else {
    buttonsSignal = 0;
  }
  switch (buttonsSignal) {
    case 5:                                         //no buttons is being pressed (divider can should be changed if button reading is wrong)
      break;
    case 4:                                         //the most left button is being pressed
      SDWriting = true;                                //enables writing to SD
      break;

    case 3:                                         //the  second button couting from the left is being pressed
      SDWriting = false;                                //enables writing to SD
      break;

    case 2:                                         //the down button is being pressed
      writeInterval = intervalMode1;                    //set writte interval to intervalMode1(5s in this case)
      timeModeLCD = 1;
      break;
    case 1:                                         //the up button is being pressed
      writeInterval = intervalMode2;                    //set writte interval to intervalMode2(1m in this case)
      timeModeLCD = 2;
      break;
    case 0:                                         //the right button is being pressed
      writeNote = true;
      break;
  }


  if (Serial) {
    Serial.print("SDWriting-"); ///help for troubleshooting
    Serial.println(SDWriting); ///help for troubleshooting
    Serial.print("SDFoundfile-"); ///help for troubleshooting
    Serial.println(SDFoundfile); ///help for troubleshooting
    Serial.print("writeInterval-"); ///help for troubleshooting
    Serial.println(writeInterval); ///help for troubleshooting
    Serial.print("timeModeLCD"); ///help for troubleshooting
    Serial.println(timeModeLCD); ///help for troubleshooting
    Serial.print("button "); ///help for troubleshooting
    Serial.println(buttonsSignal ); ///help for troubleshooting
    Serial.print("writeNote "); ///help for troubleshooting
    Serial.println(writeNote ); ///help for troubleshooting
    //Serial.print("SD.begin "); ///help for troubleshooting
    //Serial.println(SD.begin() ); ///help for troubleshooting
    Serial.println(" "); ///help for troubleshooting
  }



  if (SDWriting) {                                                  //verify if it has permission to write
    if (millis() - SDTimer > writeInterval) {
      SDTimer = millis();                                            //load the millis time to the SDTimer
      CO2datafile = SD.open("CO2data.TXT", FILE_WRITE);              //open file for writing
      if (writeNote) {
        CO2datafile.println("------------------------------------");        //note that'll be written to SD file
        writeNote = false;                                                  //reset node warning (note has been written)
      }
      CO2datafile.println(concentration);                          //SD write CO2 concentration
      fileSize = CO2datafile.size();                               //read file size
      Serial.println(CO2datafile.size()); ///help for troubleshooting
      CO2datafile.close();                                         //close file after writing to prevent corruption
    }
  }

}


// subroutines for PM2.5 sensor

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
