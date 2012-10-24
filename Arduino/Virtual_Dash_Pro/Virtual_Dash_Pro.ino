/*
  Thermistor LED (Low and High) and serial output
  Reads USP11492 10k Thermistor
  Oil Pressure LED (Low) and serial output
  Reads SSI Technologies P51-100-G-B-I36
 
 */
#include <SoftwareSerial.h>
#include <math.h>

//Setup SoftwareSerial
SoftwareSerial mySerial(8, 7);

// Setup Constants
const int sensorTempPin = A5; //Oil Temperature Sensor
const int sensorCTSPin = A4; //Coolant Temperature Sensor
const int sensorPresPin = A3; //Pressure Sensor
const int sensorRPMPin = A1; //Engine RPM
const int sensorVSSPin = A0; //VSS
const int ledPinOilLow = 12; //Low Oil Temp
const int ledPinOilHigh = 11; //High Oil Temp
const int ledPinPressureLow = 13; //Low Oil Pressure
const float Vref = 4.95; //Reference Voltage
const int loadR = 5600; //Load Resistance (Ohms)
const int tempAlarmHigh = 225; //High Temp Alarm (degF)
const int tempAlarmLow = 100; //Low Temp Alarm (degF)
const int presAlarmLow = 13; //Low Temp Alarm (psi)
const int ctsAlarmHigh = 230; //High CTS Alarm (F)
const int rpmFactor = 1; //Number of triggers per revolution - RPM
const int vssFactor = 9; //Number of triggers per revolution - VSS

volatile byte rpmCount;
unsigned int RPM;
unsigned long timeoldRPM;
volatile byte vssCount;
unsigned int VSS;
unsigned long timeoldVSS;

// Steinhart-Hart Constants
const float A = 0.0013; //Steinhart Const A
const float B = 2.1007e-4; //Steinhart Const B
const float C = 1.5215e-7; //Steinhart Const C

// Setup
void setup() {
  mySerial.begin(9600);
  pinMode(ledPinOilLow, OUTPUT);
  pinMode(ledPinOilHigh, OUTPUT);
  pinMode(ledPinPressureLow, OUTPUT);
  attachInterrupt(sensorRPMPin, countRPM, RISING);
  attachInterrupt(sensorRPMPin, countVSS, RISING);
  rpmCount = 0;
  RPM = 0;
  timeoldRPM = 0;
  vssCount = 0;
  RPM = 0;
  timeoldVSS = 0;
}

void loop() {
  char presRawADC;
  float P;
  char tempRawADC;
  float R;
  float Temp;
  char ctsRawADC;
  float CTS;
  
  tempRawADC = analogRead(sensorTempPin);
  R = loadR*(Vref - tempRawADC*5/1024)/(tempRawADC*5/1024);
  Temp = log(R);
  Temp = (1 / (A + (B * Temp) + (C * (Temp * Temp * Temp)))) - 273.15;
  Temp = (Temp * 9 / 5) + 32.0; //Convert to F
  
  presRawADC = analogRead(sensorPresPin);
  P = (25*(presRawADC-102)*5/1024);
  
  ctsRawADC = analogRead(sensorCTSPin);
  CTS = 30;
  
  // Failsafes
  if (presRawADC < 92) {
    P = -999;
  }

  if (tempRawADC < 21) {
    Temp = 999;
  }
  
  if (ctsRawADC < 21) {
    CTS = 999;
  }
  
  // Get rid of instabilitiy at 0
  if (P < 0 && P > -1) {
    P = 0;
  }
  
  // Normal Operation
  if (Temp > tempAlarmLow && Temp < tempAlarmHigh) {
    digitalWrite(ledPinOilLow, HIGH);
    digitalWrite(ledPinOilHigh, HIGH);
  }
  
  // Low Oil Temp
  if (Temp <= tempAlarmLow) {
    digitalWrite(ledPinOilLow, LOW);
    digitalWrite(ledPinOilHigh, HIGH);
  }
  
  // High Oil Temp
  if (Temp >= tempAlarmHigh) {
    digitalWrite(ledPinOilLow, HIGH);
    digitalWrite(ledPinOilHigh, LOW);
    if (CTS >= ctsAlarmHigh) {
      // High Oil AND CTS
      digitalWrite(ledPinOilLow, LOW);
    }
  }
  
  // Low Oil Pressure
  if (P <= presAlarmLow) {
    digitalWrite(ledPinPressureLow, HIGH);
  }
  else {
    digitalWrite(ledPinPressureLow, LOW);
  }
  
  if (rpmCount >= 50) { 
     RPM = rpmCount/rpmFactor*60*1000/(millis() - timeoldRPM);
     timeoldRPM = millis();
     rpmCount = 0;
  }
  
  if (vssCount >= 10) { 
     VSS = vssCount/rpmFactor*60*1000/(millis() - timeoldVSS);
     timeoldVSS = millis();
     vssCount = 0;
  }
  
  mySerial.print(Temp,1); //Oil Temperature
  mySerial.print(",");
  mySerial.print(P,1); //Pressure
  mySerial.print(",");
  mySerial.print(CTS,1); //Coolant Temperature
  mySerial.print(",");
  mySerial.print(RPM); //RPM
  mySerial.print(",");  
  mySerial.print(VSS); //VSS
  mySerial.print(",");  
  mySerial.print(rpmCount); //VSS
  mySerial.print(",");  
  mySerial.println(vssCount); //VSS
  
}

void countRPM() {
 rpmCount++; 
}

void countVSS() {
 vssCount++; 
}

