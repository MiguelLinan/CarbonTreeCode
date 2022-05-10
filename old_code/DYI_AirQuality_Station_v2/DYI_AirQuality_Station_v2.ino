/*******************************************************
Medidor ambiente
V1
2020/03/20
********************************************************/

#include <LiquidCrystal.h>

int NO2_In = A1;                            //NO2 sensor pin
int CO_In = A2;                          //CO sensor pin
int CO2_In = A3;                         //CO2 sensor pin

float raw_NO2 = 0;                      //variable to store the value comming from the analog pin
float NO2_R = 0;                        //NO2 Sensor Resistance
float NO2_ppb = 0;                      //ppb NO2 value

int raw_CO2 = 0;                         //variable to store the value coming from the analog pin
int raw_CO = 0;                          //variable to store the value coming from the analog pin

#define LENG 31                          //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;                        //define PM1.0 value of the air detector module
int PM2_5Value=0;                       //define PM2.5 value of the air detector module
int PM10Value=0;                        //define PM10 value of the air detector module


LiquidCrystal lcd(8, 9, 4, 5, 6, 7);     //select the pins used on the LCD panel

unsigned long ecra1Timer = 0;            //variable for the time ON first screen
unsigned long ecra2Timer = 0;            //variable for the time ON second screen
unsigned long ecra3Timer = 0;            //variable for the time ON third screen
unsigned long intervalo = 2000;          //variable for time exhibition between screens
bool ecra1 = true;                       //variable to force first screen
bool ecra2 = false;                      //variable to force second screen
bool ecra3 = false;                      //variable to force third screen

void setup(){
  lcd.begin(16, 2);                      //start the LCD library
  lcd.clear();                           //clean the display before write
  lcd.print("Medidor Ambiente");         //message to show
  delay(intervalo);                      //time to wait so you can read the message

  analogReference(DEFAULT);              //set the default voltage for reference voltage
  
  Serial.begin(9600);                    //start the Serial comunication with the PM2.5
  Serial.setTimeout(1500);               //if it does not start ignore
}

void loop(){
  raw_NO2 = analogRead(NO2_In);          //read the analog in value
  float NO2_Volt = raw_NO2/409.2;       //conversion formula
  NO2_R = 22000/((5/NO2_Volt) - 1);      //find sensor resistance from No2_Volt, using 5V input & 22kOhm load resistor
  NO2_ppb = (.000008*NO2_R - .0194)*1000;//convert Rs to ppb (parts per billion) concentration of NO2  
  
  raw_CO2 = analogRead(CO2_In);          //read the analog in value
  double CO2_Volt = raw_CO2 * (5000/1024.0);//conversion formula

  raw_CO = analogRead(CO_In);          //read the analog in value
  double CO_ppm = 3.027*exp(1.0698*(raw_CO * (5.0/1024)));//conversion formula

  if(Serial.find(0x42)){              //look for the 0x42 in the Serial port
    Serial.readBytes(buf,LENG);       //load the buf with the values

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value = transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value = transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value = transmitPM10(buf); //count PM10 value of the air detector module
      }
    }
  }
  
  if((millis() - ecra1Timer > intervalo) && ecra1){
    ecra2Timer = millis();                          //load the millis time to the ecra2Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = false;
    ecra2 = true;
    ecra3 = false;
    if(CO2_Volt == 0){
      // print the results to the lcd
      lcd.print("CO2: ");                             //message to show
      lcd.print("Fault");                             //message to show
    }
    if(CO2_Volt < 400){
      // print the results to the lcd
      lcd.print("CO2: ");                             //message to show
      lcd.print("preheating");                        //message to show
    }
    else{
      int voltage_diference = CO2_Volt - 400;
      float concentration = voltage_diference * (50.0/16.0); //auxiliar calculation
      lcd.print("CO2: ");                             //message to show
      lcd.print(concentration);                       // print the results to the lcd
      lcd.print(" ppm");                               //message to show
    }

    lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
    lcd.print("CO: ");                              //message to show
    lcd.print(CO_ppm);                              //print the results to the lcd
    lcd.print(" ppm");                               //message to show
   }

  if((millis() - ecra2Timer > intervalo) && ecra2){
    ecra3Timer = millis();                          //load the millis time to the ecra3Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = false;
    ecra2 = false;
    ecra3 = true;    
    lcd.print("PM1.0: ");                           //message to show
    lcd.print(PM01Value);                           //print the results to the lcd 
    lcd.print(" ug/m3");                             //message to show
    
    lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
    lcd.print("PM2.5: ");                           //message to show
    lcd.print(PM2_5Value);                          //print the results to the lcd
    lcd.print(" ug/m3");                             //message to show
   }
  
  if((millis() - ecra3Timer > intervalo) && ecra3){
    ecra1Timer = millis();                          //load the millis time to the ecra1Timer variable
    lcd.clear();                                    //clean the display before write
    ecra1 = true;                                   //message to show
    ecra2 = false;
    ecra3 = false;
    lcd.print("PM10: ");                            //message to show
    lcd.print(PM10Value);                           //print the results to the lcd 
    lcd.print(" ug/m3");                             //message to show

    lcd.setCursor(0, 1);                            //set the LCD cursor position first column(0) and second line(1)
    lcd.print("NO2: ");                             //message to show
    lcd.print(NO2_ppb);                             //print the results to the lcd
    lcd.print(" ppb");                               //message to show
   }
}

// subroutines for PM2.5 sensor

char checkValue(unsigned char *thebuf, char leng)
{
  char receiveflag = 0;
  int receiveSum = 0;

  for(int i=0; i<(leng-2); i++){
  receiveSum = receiveSum + thebuf[i];
  }
  receiveSum = receiveSum + 0x42;

  if(receiveSum == ((thebuf[leng-2]<<8) + thebuf[leng-1]))  //check the serial data
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val = ((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val = ((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val = ((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module
  return PM10Val;
}
