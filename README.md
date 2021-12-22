# Remind-Me-Stuff

## 1. TLDR

This is a proof of concept that is work in progress. Final product is intended to run on a microcontroller connected to a NeoKey 5x6 custom mechanical keyboard where all keys are tasks and/or reminders set up by the user. to remind the user to do simple stuff like drink water, open a window becuse the temperature in the room is too hot.

## 2. Usecase

The usecase is kind of simple, display a led on a key that reminds you of simple tasks/reminder, these reminders can be everything from Drinking water, changing bedsheets, water the flowers etc. and to turn of these reminders you would need to press the dedicated key on the keyboard to indicate that you have infact done the task.

It is also suposed to get some sort of sensor data such as temperature from a DHT11 sensor and if it gets too hot where that sensor is placed, it is suposed to light up a key on the keyboard the same way as the reminder to drink water. Only differance beeing that you cant complete this "task" by yourself, you would need to open a door or turn on a fan to lower the temperature in the room so the sensor would detect that it is not as hot anymore and turn of the warning.


## 3. Solution

### 3.1 System Diagram
This is the current system diagram since the NeoKey custom keyboard has not arived yet... :/
![](/Architecture.png)

I chose to use Firebase since it is quick to set up and to get started with.

### 3.2 Hardware
Currently I only have two simple LEDs, A DHT11 sensor, two buttons and it all is connected to a ESP8266.

### 3.3 Firebase
I store all my data in Firestore, Firestore database is a noSQL document database. It has built in live synchronization, and is intended for "Hot storage". Firestore is part of google cloud.

The data in firebase looks like this:
![](/Dataflow1.png)

To  acces Firebase/Firestore using the ESP i used a library called [Firebase-ESP-Client by Mobizt](https://github.com/mobizt/Firebase-ESP-Client).

I send data to Firebase, Firebase functions add a timestamp to the data and does some calculation based on the data. See Cloud Functions.

I host a temporary Website using Firebase hosting.

### 4. Guides/helpful things i have used during this project
* https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-sensor-arduino-ide/
* https://github.com/mobizt/Firebase-ESP-Client
* https://github.com/firebase/functions-samples

### 5. Other links I plan on using in this project
* https://www.adafruit.com/product/5157 
* https://learn.adafruit.com/deluxe-4x4-neopixel-neokey-keypad