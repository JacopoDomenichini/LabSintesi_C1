#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include "EasyBuzzer.h"
#include <TimeLib.h>
#include <CapTouch.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>

//Display SevSeg
Adafruit_7segment matrix = Adafruit_7segment();
unsigned long timer = millis();
int StartTime = 240;
int Seconds = StartTime + 1;
int Min = 0;
int Sec = 0;
int MinSec = 0;
int n = 0;
int Digit_0 = 0, Digit_1 = 0, Digit_2 = 0, Digit_3 = 0;
int BrightnessCnt = 1;
int Brightness = 0;
int Scrolling = 1;

//Buzzer
unsigned long BuzzTimer = 0;
int BombTick = 0;
int buzzerPin = 5;

//Ultrasonic
#define trigPin 4                     // Pin 4 trigger output
#define echoPin 3                     // Pin 3 Echo input
#define TICK_COUNTS 200               // 200 counts x 120us of timer ticks
volatile long echo_start = 0;         // Records start of echo pulse
volatile long echo_end = 0;           // Records end of echo pulse
volatile long echo_duration = 0;      // Duration - difference between end and start
volatile int trigger_time_count = 0;  // Count down counter to trigger pulse time
float Sum = 0;                        // Sums
float Average = 0;
float DistanceCm = 0;
float DistMax = 120;
float DistMin = 30;
unsigned long AvgTimer = 0;
int i = 0;



//CapacitiveSensor
#define CapSendPin 11    // Pin A0 Capacitive Sensor Send Pin 10Mohm
#define CapRecivePin 10  // Pin A1 Capacitive Sensor Receive Pin (to foil plate)
// CapTouch(sendPin, receivePin) - recieve pin is the sensor to touch
CapTouch TouchSensor = CapTouch(CapSendPin, CapRecivePin);
long TouchSensorLevel = 0;
int TouchSensorLevel_i = 0;
long TouchSensorLevel_avg = 0;
long TouchSensorLevel_sum = 0;
bool TouchSensorState = 0;
bool TouchSensorOneShot = 0;

//Cooling  Fan
int CoolingFanPin = 9;  //Output number for N-Mosfet to control Cooling Fan
int CoolingFanState = 0;
long CoolingFanTimer = 0;

//PeltierCell
int PeltierOutputBit = 6;    //Output number for N-Mosfet to control peltier cell
int PeltierOutputLevel = 0;  // peltier cell power level from 0 to 255
long int PeltierTimer = 0;

//WIFI
int status = WL_IDLE_STATUS;
char ssid[] = "bomb";                // your network SSID (raspberry pi hotspot)
char pass[] = "12345678";            // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                    // your network key index number (needed only for WEP)
unsigned int localPort = 2390;       // local port to listen on (arduino)
IPAddress localIP(192, 168, 4, 2);   // local ip adress (arduino)
unsigned int RemotePort = 5005;      //remote port (raspberry)
IPAddress remoteIP(192, 168, 4, 1);  //remote ip adress (raspberry)
char packetBuffer[1024];             //buffer to hold incoming packet
char UDP_DataToSend[] = "pippo";     // a string to send back
WiFiUDP Udp;

bool StopForVideo = false;
int AttemptConnect = 0;
bool NoWifi = 0;
bool DataSentFlag = 0;
long TimerToRestart = 0;
String finished = "finished";
String UDP_DataReceived = "";
long WifiAttempt_ms = 0;

// Utilities
unsigned long scanTime_us = 0;
unsigned long LastMicroseconds = 0;
int Jumper1Pin = 8;
int Jumper2Pin = 7;
bool Jumper1Present = 0;
bool Jumper2Present = 0;



// Setup() Routine executed once at system startup
void setup() {

  delay(1000);

  //Display SevSeg
  matrix.begin(0x70);  //i2c address for adafruit led backpack


  //Buzzer
  EasyBuzzer.setPin(buzzerPin);  //uso pin 1 per il buzzer
  BuzzTimer = millis();

  //Ultrasonic
  pinMode(trigPin, OUTPUT);        // Trigger pin set to output
  pinMode(echoPin, INPUT_PULLUP);  // Echo pin set to input

  InternalRTC.attachInterrupt(timerIsr, 8192);  // set user timerIsr function fired by internal periodic interrupt (PIT)
                                                // (1Hz by defaut, max 8192Hz, only power of 2 number)
                                                // run only on megaAVR-0 series!
                                                // 8192-> 1sec/8192=122usec period

  attachInterrupt(echoPin, echo_interrupt, CHANGE);  // Attach interrupt to the sensor echo input


  //PeltierCell
  pinMode(PeltierOutputBit, OUTPUT);

  //Cooling Fan
  pinMode(CoolingFanPin, OUTPUT);

  //Ultrasonic
  pinMode(Jumper1Pin, INPUT_PULLUP);  // Trigger pin set to output
  pinMode(Jumper2Pin, INPUT_PULLUP);  // Echo pin set to input

  //WIFI
  Serial.begin(9600);  // Open the serial port
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED && !NoWifi) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    Serial.println(AttemptConnect);
    WiFi.config(localIP);
    status = WiFi.begin(ssid, pass);
    WifiAttempt_ms = millis() + 5000;
    while (millis() < WifiAttempt_ms) {
      Circles();
    }
    AttemptConnect = AttemptConnect + 1;
    if (AttemptConnect >= 3) {
      NoWifi = true;
    }
  }
  if (!NoWifi) {
    Serial.println("Connected to WiFi");
    Udp.begin(localPort);  // if you get a connection send message via serial:
    Yes();
    delay(2000);
  }
  if (NoWifi) {
    Serial.println("WiFi not available");
    no();
    delay(2000);
  }
}

//Routine Loop() executed cyclically
void loop() {

  //Utilities
  // utility to read the loop() routine scantime
  scanTime_us = micros() - LastMicroseconds;
  LastMicroseconds = micros();
  //Jumpers presence monitor
  Jumper1Present = !digitalRead(Jumper1Pin);
  Jumper2Present = !digitalRead(Jumper2Pin);
  //Serial.print(scanTime_us);


  //Display SevSeg
  if (millis() >= 1000) {
    if (millis() - BuzzTimer >= BombTick && !TouchSensorState && !StopForVideo) {
      timer += 1000;
      Seconds--;  // 100 milliSeconds is equal to 1 deciSecond


      if (Seconds == 0) {     // Reset to 0 after counting for 1000 seconds.
        StopForVideo = true;  // Bomb Timer elapsed!!!!!!!!!!!!!!!
        Seconds = StartTime;
        n = 0;
      }

      Min = abs(Seconds / 60);
      Sec = Seconds - (Min * 60);
      MinSec = (Min * 100) + Sec;
      Digit_0 = abs(MinSec / 1000);
      Digit_1 = abs((MinSec / 100) % 10);
      Digit_2 = abs((MinSec / 10) % 10);
      Digit_3 = abs(MinSec % 10);

      //matrix.writeDigitNum(pos, val, dp);
      //matrix.drawColon(x);
      matrix.writeDigitNum(0, Digit_0, 0);
      matrix.writeDigitNum(1, Digit_1, 0);
      matrix.drawColon(1);
      matrix.writeDigitNum(3, Digit_2, 0);
      matrix.writeDigitNum(4, Digit_3, 0);
      matrix.writeDisplay();
    }
  }

  //Buzzer
  if (millis() - BuzzTimer >= BombTick && !TouchSensorState && !StopForVideo) {
    BuzzTimer = millis();
    EasyBuzzer.singleBeep(
      1700,  // Frequency in hertz(HZ).
      50     // Duration of the beep in milliseconds(ms).
    );
  }
  // Always call this function in the loop for EasyBuzzer to work.
  EasyBuzzer.update();



  //Ultrasonic sensor
  DistanceCm = echo_duration / 58;
  if (DistanceCm > 120) {
    DistanceCm = 120;
  }
  if (DistanceCm < 30) { 
    DistanceCm = 30;
  }

  i = i + 1;
  Sum = Sum + DistanceCm;
  if (millis() - AvgTimer > 500) {
    Average = (Sum / i);
    BombTick = (Average - DistMin) / (DistMax - DistMin) * (1000 - 100) + 100;
    if (BombTick < 150) {
      BombTick = 150;
    }
    Sum = 0;
    i = 0;
    AvgTimer = millis();

    // print results
    // Serial.print("cm: ");
    // Serial.println(DistanceCm);
    // Serial.print(" Average = ");
    // Serial.println(BombTick);
    // Serial.print (" i");
    // Serial.print (i);
  }

  //CapacitiveSensor
  long TouchSensorLevel = TouchSensor.readTouch(4);  //  read the sensor (Samples)

  // calculates the average of the TouchSensorLevel over 30 readings
  if (TouchSensorLevel_i > 30) {
    TouchSensorLevel_avg = TouchSensorLevel_sum / TouchSensorLevel_i;

    if (TouchSensorLevel_avg > 3) {
      TouchSensorState = true;
    } else {
      TouchSensorState = false;
    }
    TouchSensorLevel_i = 0;
    TouchSensorLevel_sum = 0;
    Serial.println(TouchSensorLevel_avg);  // print sensor output 1

  } else {
    TouchSensorLevel_i++;
    TouchSensorLevel_sum = TouchSensorLevel_sum + TouchSensorLevel;
  }


  //PeltierCell
  if (TouchSensorState) {
    if (!TouchSensorOneShot) {
      TouchSensorOneShot = true;
      PeltierTimer = millis();
    }
    if (millis() - PeltierTimer < 5000) {
      PeltierOutputLevel = 95;  //when the touch sensor is on for the first five second the peltier power level is 70 on 255
    } else {
      PeltierOutputLevel = 60;  //when the touch sensor is on for longer than 5 seconds the peltier power level is turning into 35 on 255
    }
  }
  if (!TouchSensorState) {
    TouchSensorOneShot = false;
    PeltierOutputLevel = 0;
  }
  analogWrite(PeltierOutputBit, PeltierOutputLevel);
  //Serial.print(PeltierTimer);
  //Serial.println(PeltierOutputLevel);

  //Cooling Fan
  if (TouchSensorState) {  //when the TouchSensorState is pressed the Cooling Fan is started
    CoolingFanState = 1;
  }
  switch (CoolingFanState) {
    case 1:
      digitalWrite(CoolingFanPin, HIGH);
      if (!TouchSensorState) {  //when the TouchSensorState is released starts a 10s timer
        CoolingFanTimer = millis();
        CoolingFanState = 2;
      }
      break;
    case 2:
      if ((millis() - CoolingFanTimer) > 30000) {  //when the 10 seconds timer is expired the Cooling Fan is stopped
        digitalWrite(CoolingFanPin, LOW);
        CoolingFanState = 0;
      }
      break;
  }

  //WIFI
  if (StopForVideo) {  // When the bomb timer elapse everything stops
    ICE_Logo();        // print a string message
    if (!NoWifi) {
      if (!DataSentFlag) {
        UDP_Send();
        DataSentFlag = true;
      }

      // if there's data available, read a packet
      UDP_DataReceived = UDP_Receive();
      if (UDP_DataReceived == finished) {
        StopForVideo = false;
        DataSentFlag = false;
      }
    }
    if (NoWifi) {
      if (TimerToRestart == 0) {           //initially sets the timer to restat at 2 seconds
        TimerToRestart = millis() + 2000;  //With no WIFI 2 second only for ogo
      }
      if (TimerToRestart != 0 && millis() > TimerToRestart) {  //At timer to restart elapsed everything restarts
        TimerToRestart = 0;
        StopForVideo = false;
        matrix.setBrightness(15);  //restore max brightness
      }
    }
  }
}

//*******************************************************
//*******************************************************
//Loop()  continuous loop routine End
//*******************************************************



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Functions called from Interrupts (Timers or I/Os) or from main code in Loop()
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@


//timerIsr() Function triggered from RTC interrupt Timer every 1/8192sec (122usec)
void timerIsr() {
  trigger_pulse();  // calls the trigger_pulse() function to generate the trigger for the ultrasonic sensor
}

// --------------------------
// trigger_pulse() called every 122 uS to schedule trigger pulses.
// Generates a pulse one timer tick long (122usec) every 200 cycle (24,4msec).
// Minimum trigger pulse width for the HC-SR04 is 10 us. This system
// delivers a 122 uS pulse.
// --------------------------
void trigger_pulse() {
  static volatile int state = 0;  // State machine variable

  if (0 > (--trigger_time_count))      // Count to 200cycles of 122usec
  {                                    // Time out - Initiate trigger pulse
    trigger_time_count = TICK_COUNTS;  // Reload 200 in the counter
    state = 1;                         // Changing to state 1 initiates a pulse
  }

  switch (state)  // State machine handles delivery of trigger pulse
  {
    case 0:  // Normal state does nothing
      break;

    case 1:                         // Initiate pulse
      digitalWrite(trigPin, HIGH);  // Set the trigger output high
      state = 2;                    // and set state to 2
      break;

    case 2:  // Complete the pulse
    default:
      digitalWrite(trigPin, LOW);  // Set the trigger output low
      state = 0;                   // and return state to normal 0
      break;
  }
}

// --------------------------
// echo_interrupt() External interrupt from HC-SR04 echo signal.
// Called every time the echo signal changes state.
// Note: this routine does not handle the case where the timer
// counter overflows which will result in the occassional error.
// --------------------------
void echo_interrupt() {
  switch (digitalRead(echoPin))  // Test to see if the signal is high or low
  {
    case HIGH:                // High so must be the start of the echo pulse
      echo_end = 0;           // Clear the end time
      echo_start = micros();  // Save the start time
      break;

    case LOW:                                 // Low so must be the end of hte echo pulse
      echo_end = micros();                    // Save the end time
      echo_duration = echo_end - echo_start;  // Calculate the pulse duration
      break;
  }
}

//WIFI SEND DATA
void UDP_Send() {
  Udp.beginPacket(remoteIP, RemotePort);
  Udp.write(UDP_DataToSend);
  Udp.endPacket();
  Serial.println(UDP_DataToSend);
}

//WIFI RECEIVE DATA
String UDP_Receive() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);
    return packetBuffer;
  }
}


// ICE_Logo() to print the logo ICE
void ICE_Logo() {
  ChangeBrightness();
  RandomICE();
  delay(50);
  //RollingICE();
}

// Change the display brightness for the ICE logo
void ChangeBrightness() {
  if (BrightnessCnt < 30) {
    BrightnessCnt++;
    if (BrightnessCnt < 16) {
      Brightness = (BrightnessCnt / 1);
    } else {
      Brightness = (31 - BrightnessCnt / 1);
    }

  } else {
    BrightnessCnt = 1;
  }
  //Serial.println(Brightness);
  matrix.setBrightness(Brightness);
}

// Random composition ICE logo
void RandomICE() {
  delay(random(0, 40));
  int Flicker = random(1, 10);
  //Serial.println(Flicker);
  switch (Flicker)  // Test to see if the signal is high or low
  {
    case 1:  // High so must be the start of the echo pulse
      matrix.drawColon(0);
      matrix.println(" ICE");
      matrix.writeDisplay();
      break;

    case 2:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 3:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("I   ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 4:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println(" C  ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 5:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("  E ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 6:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;
    case 7:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println(" CE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;
    case 8:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("I E ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;
    case 9:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("IC  ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;
  }
}

// Random composition ICE logo
void RollingICE() {
  delay(100);

  if (Scrolling < 17) {
    Scrolling++;
  } else {
    Scrolling = 1;
  }

  Serial.println(Scrolling);
  switch (Scrolling)  // Test to see if the signal is high or low
  {
    case 1:  // High so must be the start of the echo pulse
      matrix.drawColon(0);
      matrix.println("    ");
      matrix.writeDisplay();
      break;

    case 2:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("   I");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 3:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("  IC");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 4:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println(" ICE");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 5:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 6:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 7:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 8:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 9:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 10:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 11:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 12:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 13:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 14:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 15:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("ICE ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 16:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("CE  ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;

    case 17:  // Low so must be the end of hte echo pulse
      matrix.drawColon(0);
      matrix.println("E   ");
      matrix.writeDisplay();  // Calculate the pulse duration
      break;
  }
}

void Circles() {
  static int i, d;
  uint8_t icon;
  uint8_t blank = 0;
  i++;
  if (i > 5) {
    i = 1;
    d++;
    if (d == 2) {
      d = 3;
    }
    if (d > 4) {
      d = 0;
    }
  }
  switch (i) {

    case 0:
      //segment GFEDCBA
      icon = 0b00000000;
      break;
    case 1:
      //segment GFEDCBA
      icon = 0b00000001;
      break;
    case 2:
      //segment GFEDCBA
      icon = 0b00000010;
      break;
    case 3:
      //segment GFEDCBA
      icon = 0b01000000;
      break;
    case 4:
      //segment GFEDCBA
      icon = 0b00100000;
      break;
    case 5:
      //segment GFEDCBA
      icon = 0b00000001;
      break;
  }

  matrix.println();
  matrix.writeDigitRaw(0, blank);
  matrix.writeDigitRaw(1, blank);
  matrix.writeDigitRaw(2, blank);
  matrix.writeDigitRaw(3, blank);
  matrix.writeDigitRaw(4, blank);
  matrix.writeDigitRaw(d, icon);
  matrix.writeDisplay();
  delay(100);
}

void Yes() {
  matrix.drawColon(0);
  matrix.println("YES ");
  matrix.writeDisplay();  // Calculate the pulse duration
}

void no() {
  matrix.drawColon(0);
  matrix.println("no  ");
  matrix.writeDisplay();  // Calculate the pulse duration
}
