# **CarbonTreeCode**
This repository contains all the code needed to run an Air Quality Station from the __Carbon Tree__ project!

## **Purpose**

---

An **Air Quality Station** is an _arduino_ based project which can take data from **__4 sensors__** and both **save** and **display** those values. The measured molecules/particles are:

* **CO<sub>2</sub>** (Carbon Dioxide)
* **CO** (Carbon monoxide)
* **NO<sub>2</sub>** (Nitrogen Dioxide)
* **PMs** (suspended Particulate Matter)
    * PM **1.0μm**
    * PM **2.5μm**
    * PM **10.0μm**


The data collected is **displayed** in a 2x16 **LCD display** _and_ **saved** to an **SD card** as table of values which you can open in Excel (.csv format).


## **Station Capabilities**

---

The Air Quality Station code/software has various useful features:

* Setting the **time delay** between each **measurement** from the sensors
* Setting up a **timer** for the data collection to **stop**
* The ability to **chose** which **sensors** to collect data from
* The option of choosing between saving new data on a **new file** or in an **existing one**  
* The ability of **saving** (manually) the **date** and **time** of the beginning of the experiment in the file
* Access to screen with information about the data collection process:
    * How many **bites** in the **data file**
    * How much **time left** until the data collection is **over**
    * A button which **saves a note** in the data file in that **exact moment** (as a way of taking notes of important moments which could affect the data collected) 

The **manual** on how to operate this machine can be found on the following [**Instructions**][link_tutorial]  

## **Carbon Tree**

---

### _What is the Carbon Tree Project?_

Well... 
text

.

## **Building a Station**

---

1. To build an Air Quality Station you'll need the **materials** in this 
[**Document**](https://docs.google.com/document/d/1l7tG8-l5VGNB0ed4CIbOwf0h02rkb5lN/edit?usp=sharing&ouid=109219029885592963980&rtpof=true&sd=true).

2. The **assembly** of the Station can be found on this [**Video**](https://www.youtube.com/watch?v=oLzKA4CCkwY) (subtitles available in English)

3. To install the correct **code** on the Station/Arduino you'll need to install the arduino IDE which can be downloaded from here: [**Arduino_IDE_Link**](https://www.arduino.cc/en/software).

4. After you've installed the Arduino IDE you'll have to **download** and **upload** the **code** to **Arduino** inside the Station.
The most recent version of the code can be found right above this README.md with a name like: `X.X.X_AirQuality_Station.ino` (X being the version of the code). For further information you can visit this [page](https://learn.adafruit.com/ladyadas-learn-arduino-lesson-number-1/upload-your-first-sketch).

If you want to learn how to use your new Station please take a look at the following [**Instructions**][link_tutorial]   

## **Using The Station**

---

To **simplest way** to use the **Station** is by selecting `1-Endless` -> `1-Default` ->  `1-All` -> `1-New` -> `2-Skip`  as soon as it start, making sure the SD car is in the SD reader slot below the LCD. By selecting these options the data collection process will start in a `new` file with a time between data measurements of 1 minute (the `default`). It will do so for an `endless` period of time (actually 49 days) taking data from `all` sensors. The start date and time will not be saved in the file because of the `skip` done in the setup.

To take advantage of the full functionalities of the Station like saving the date and time in the file itself please go through the [**Instructions**][link_tutorial]  


[link_tutorial]: https://docs.google.com/document/d/1T4xtQLqhaCJcnTU4sV4L8lUPmjTFFTN8yCQhNOLety0/edit?usp=sharing     **_<- Insert link_**



