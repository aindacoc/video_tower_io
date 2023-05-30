/*
  Program title: Video Wall I/O (Arduino 2)
  Description: Handles sensor and potentiometer input and controls Dataprobe K415 Power Strip
  Created by: Andres Indacochea
  Created on: 4/26/23
  Last Modified by: Andres Indacochea
  Last Modified on: 5/25/23
*/

// Import Ultrasonic sensor library by gamegine
#include <HCSR04.h>

struct PIR {
  const byte Pin;
  unsigned int CycleTrack;
  bool SerialSent;
  bool Advanced;
  bool Powered;
};

struct Pot {
  const int Pin;
  bool SerialSent;
  bool Sampled;
  unsigned long SampleTrack;
  unsigned int ConvertedReading;
  byte hi;
  byte lo;
};

// Setup Hardware Pins, DO NOT CHANGE UNLESS HARDWARE CHANGES ARE MADE
Pot Pot1 = {A6};      //  "Pot1" connected to pin A6
PIR PIR1 = {13};     //  "Pir1" connected to pin D13
const byte Power1 = 12;   //  "Power1" connected to pin D12
const byte Power2 = 11;   //  "Power2" connected to pin D11
const byte Power3 = 10;   //  "Power3" connected to pin D10
HCSR04 Ultra1(9, 8);      //  "Ultra1" TRIG connected to pin D9 and ECHO connected to pin D8
HCSR04 Ultra2(7, 6);      //  "Ultra1" TRIG connected to pin D7 and ECHO connected to pin D6
HCSR04 Ultra3(5, 4);      //  "Ultra1" TRIG connected to pin D5 and ECHO connected to pin D4
HCSR04 Ultra4(3, 2);      //  "Ultra1" TRIG connected to pin D3 and ECHO connected to pin D2
HCSR04 Ultra5(A1, A0);    //  "Ultra1" TRIG connected to pin A1 and ECHO connected to pin A0

// Global variables for tracking ultrasonics
bool Ultra1ReadTrack = LOW;
unsigned long Ultra1SampleTrack = 0;
bool Ultra2ReadTrack = LOW;
unsigned long Ultra2SampleTrack = 0;
bool Ultra3ReadTrack = LOW;
unsigned long Ultra3SampleTrack = 0;
bool Ultra4ReadTrack = LOW;
unsigned long Ultra4SampleTrack = 0;
bool Ultra5ReadTrack = LOW;
unsigned long Ultra5SampleTrack = 0;


void ReadPot (Pot *ofInt, byte SampleRate) {
  if (ofInt->Sampled == LOW) {
    ofInt->ConvertedReading = (analogRead(ofInt->Pin) / 4.01);
    ofInt->hi = highByte(ofInt->ConvertedReading);
    ofInt->lo = lowByte(ofInt->ConvertedReading);
    byte toSend[2] = {ofInt->hi, ofInt->lo};
    Serial.write(toSend, 2 );
    ofInt->Sampled = HIGH;
  } else {
    if (ofInt->SampleTrack >= 4000000) {
      ofInt->SampleTrack = 0;
      ofInt->Sampled = LOW;
    } else {
      ofInt->SampleTrack = ofInt->SampleTrack + SampleRate;
    }
  }
}

void ReportPositiveThresholdPot (Pot *ofInt, byte ThresholdValue, unsigned int MessageToSend) {
  if (ofInt->SerialSent == LOW && ofInt->ConvertedReading > ThresholdValue) {
    Serial.write(MessageToSend);
    ofInt->SerialSent = HIGH;
  }
  else if (ofInt->ConvertedReading < ThresholdValue) {
    ofInt->SerialSent = LOW;
  }
}

void ReportNegativeThresholdPot (Pot *ofInt, byte ThresholdValue, unsigned int MessageToSend) {
  if (ofInt->SerialSent == LOW && ofInt->ConvertedReading < ThresholdValue) {
    Serial.write(MessageToSend);
    ofInt->SerialSent = HIGH;
  }
  else if (ofInt->ConvertedReading > ThresholdValue) {
    ofInt->SerialSent = LOW;
  }
}

void SerialOnPIR (PIR * ofInt, byte MessageToSend) {
  if (digitalRead(ofInt->Pin) == LOW) {     // Detect initial change in PIR status
    if (ofInt->SerialSent == LOW) {                // No event if State is already high
      ofInt->SerialSent = HIGH;                    // Set State high to indicate detection
      Serial.write(MessageToSend);            // Print confirmation message
    }
  } else {
    ofInt->SerialSent = LOW;                     // Set State low if PIR already detected
  }
}

void CycledPowerOffPIR (byte PowerControl, PIR * PIROI, unsigned int PowerOffCycles) {
  if (digitalRead(PIROI->Pin) == LOW && PIROI->Powered == LOW) {
    PIROI->Powered = HIGH;
    digitalWrite(PowerControl, HIGH);
  }
  if (PIROI->Powered == HIGH) {
    if (PIROI->CycleTrack >= PowerOffCycles) {
      PIROI->CycleTrack = 0;
      PIROI->Powered = LOW;
      digitalWrite(PowerControl, LOW);
    } else {
      PIROI->CycleTrack = PIROI->CycleTrack + 1;
    }
  }
}

void CycledPowerOnPIR (byte PowerControl, PIR * PIROI, unsigned int PowerOnCycles) {
  if (digitalRead(PIROI->Pin) == LOW && PIROI->Powered == LOW) {
    PIROI->Powered = HIGH;
    digitalWrite(PowerControl, LOW);
  }
  if (PIROI->Powered == HIGH) {
    if (PIROI->CycleTrack >= PowerOnCycles) {
      PIROI->CycleTrack = 0;
      PIROI->Powered = LOW;
      digitalWrite(PowerControl, HIGH);
    } else {
      PIROI->CycleTrack = PIROI->CycleTrack + 1;
    }
  }
}

void setup() {

  Serial.begin(9600);      //  Begin serial communication at 9600 baud rate

  // Declare pins as inputs or outputs
  pinMode(Pot1.Pin, INPUT);    //  "Pot1" is an input
  pinMode(PIR1.Pin, INPUT);    //  "Pir1" is an input
  pinMode(Power1, OUTPUT); //  "Power1" is an output
  pinMode(Power2, OUTPUT); //  "Power2" is an output
  pinMode(Power3, OUTPUT); //  "Power3" is an output
  // No need to configure ultras, library does it automatically

}

void loop() {
  ReportPositiveThresholdPot(&Pot1, 200, 420);
}

/*---- NOTES, EXTRA CODE, ETC. -----//

// Ultra read block

  if (Ultra1ReadTrack == LOW) {
    unsigned int ConvertedReading = (Ultra1.dist());
    Serial.println(ConvertedReading);
    Ultra1ReadTrack = HIGH;
  } else {
    if (Ultra1SampleTrack >= 4000000) {
      Ultra1SampleTrack = 0;
      Ultra1ReadTrack = LOW;
    } else {
      Ultra1SampleTrack = Ultra1SampleTrack + 100;
    }
  }
*/
