/*
  Program title: Video Wall I/O
  Description: Handles sensor, button, and switch I/O and controls Dataprobe K415 Power Strip
  Created by: Andres Indacochea
  Created on: 4/26/23
  Last Modified by: Andres Indacochea
  Last Modified on: 5/25/23
*/

// Create structure for Light up Buttons
struct LBut {               // Name of structure is "LBut"
  const int ButPin;         // "LBut" has an associated pin for button input
  const byte LEDPin;        // "LBut" has an assocaited pin for LED output
  int BlinkTrack;           // Variable to track time for blinking
  unsigned int FadeTrack;   // Variable to track time and set LED brightness for fading
  unsigned int FreezeTrack; // Variable to track time for freezing button state
  bool TrackDirection;      // Variable to switch direction of tracks
  bool ButtonPress;         // Variable to detect and hold button presses
  bool StateChange;         // Variable to signal the change in state
  bool Frozen;              // Variable to store freeze state of button
  bool SkipMe;              // Variable to skip other lighting routines during freezing
  bool Debounce;            // Variable to hold button input for debounce
  byte DebounceTrack;       // Variable to track the number of cycles to debounce for
  byte State;               // Variable to hold the state of the button
};

// Setup Hardware Pins, DO NOT CHANGE UNLESS HARDWARE CHANGES ARE MADE
const int Pot2 = A6;        // "Pot2" connected to pin A6
LBut LBut1 = {A0, 5};       // "LBut1" connected to pin A0, LED connected to pin D5
LBut LBut2 = {A1, 6};       // "LBut2" connected to pin A1, LED connected to pin D6
LBut LBut3 = {2, 9};        // "LBut3" connected to pin D2, LED connected to pin D9
LBut LBut4 = {3, 10};       // "LBut4" connected to pin D3, LED connected to pin D10
LBut LBut5 = {4, 11};       // "LBut5" connected to pin D4, LED connected to pin D11
const byte PIR2 = 7;        // "PIR2" connected to pin D7
const byte Power1 = 8;      // "Power1" connected to pin D8
const byte Power2 = 12;     // "Power2" connected to pin D12
const byte Power3 = 13;     // "Power3" connected to pin D13


void TrackButton (LBut *ofInt, byte NumberOfStates) {
  if (digitalRead(ofInt->ButPin) == LOW) {
    if (ofInt->ButtonPress == LOW && ofInt->Debounce == LOW) {
      ofInt->ButtonPress = HIGH;
      ofInt->StateChange = HIGH;
      ofInt->Frozen = HIGH;
      ofInt->Debounce = HIGH;
      ofInt->State = ofInt->State + 1;
      if (ofInt->State >= NumberOfStates) {
        ofInt->State = 0;
      }
    }
  } else if (ofInt->Debounce == LOW) {
    ofInt->ButtonPress = LOW;
    ofInt->Debounce = HIGH;
  }
  if (ofInt->Debounce == HIGH) {
    ofInt->DebounceTrack = ofInt->DebounceTrack + 1;
    if (ofInt->DebounceTrack >= 30) {
      ofInt->Debounce = LOW;
      ofInt->DebounceTrack = 0;
    }
  }
}


void FreezeState(LBut *ofInt, unsigned int FreezeTime) {
  if (ofInt->Frozen == HIGH) {
    if (ofInt->ButtonPress == LOW) {
      ofInt->ButtonPress = HIGH;
      ofInt->SkipMe = HIGH;
      ofInt->FreezeTrack = ofInt->FreezeTrack + 1;
      if (ofInt->FreezeTrack >= FreezeTime) {
        ofInt->ButtonPress = LOW;
        ofInt->SkipMe = LOW;
        ofInt->FreezeTrack = 0;
        ofInt->Frozen = LOW;
      }
    }
    delay(1);
  }
}


void ReportStateChange (LBut *ofInt, byte MessageToSend) {
  if (ofInt->StateChange == HIGH) {
    ofInt->StateChange = LOW;
    Serial.println(MessageToSend);
  }
}

void LEDOn (LBut *ofInt) {
  if (ofInt->SkipMe == LOW) {
    digitalWrite(ofInt->LEDPin, HIGH);
  }
}

void LEDOff (LBut *ofInt) {
  if (ofInt->SkipMe == LOW) {
    digitalWrite(ofInt->LEDPin, LOW);
  }
}


void Blink (LBut *ofInt, byte BlinkSpeed) {
  if (ofInt->SkipMe == LOW) {  
    ofInt->BlinkTrack = ofInt->BlinkTrack + BlinkSpeed;
    if (ofInt->BlinkTrack < 0) {
      analogWrite(ofInt->LEDPin, 0);
    } else {
      analogWrite(ofInt->LEDPin, 255);
    }
    delay(1);
  }
}

void FadeInOut (LBut *ofInt, byte FadeSpeed) {
  if (ofInt->SkipMe == LOW) {
    if (ofInt->FadeTrack <= 255) {
      ofInt->TrackDirection = HIGH;
      ofInt->FadeTrack = ofInt->FadeTrack + FadeSpeed;
    } else if (ofInt->FadeTrack >= 64770) {
      ofInt->TrackDirection = LOW;
      ofInt->FadeTrack = ofInt->FadeTrack - FadeSpeed;
    }
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack)/253));
    if (ofInt->TrackDirection == HIGH) {
      ofInt->FadeTrack = ofInt->FadeTrack + FadeSpeed;
    } else {
      ofInt->FadeTrack = ofInt->FadeTrack - FadeSpeed;
    }
    delay(1);
  }
}

void FadeAsc (LBut *ofInt, byte FadeSpeed) {
  if (ofInt->SkipMe == LOW) {
    ofInt->FadeTrack = ofInt->FadeTrack + FadeSpeed;
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack)/255));
    delay(1);
  }
}

void FadeDes (LBut *ofInt, byte FadeSpeed) {
  if (ofInt->SkipMe == LOW) {
    ofInt->FadeTrack = ofInt->FadeTrack - FadeSpeed;
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack)/255));
    delay(1);
  }
}

void SwitchOn (byte PowerControl) {
  digitalWrite(PowerControl, LOW);
}

void SwitchOff (byte PowerControl) {
  digitalWrite(PowerControl, HIGH);
}



void setup() {

  Serial.begin(9600);        // Begin serial communication at 9600 baud rate

  // Declare pins as inputs or outputs
  pinMode(Pot2, INPUT);      // "Pot2" is an input
  pinMode(PIR2, INPUT);      // "PIR2" is an input
  pinMode(LBut1.ButPin, INPUT);     // "LBut1" is an input
  pinMode(LBut2.ButPin, INPUT);     // "LBut2" is an input
  pinMode(LBut3.ButPin, INPUT);     // "LBut3" is an input
  pinMode(LBut4.ButPin, INPUT);     // "LBut4" is an input
  pinMode(LBut5.ButPin, INPUT);     // "LBut5" is an input
  pinMode(Power1, OUTPUT);   // "Power1" is an output
  pinMode(Power2, OUTPUT);   // "Power2" is an output
  pinMode(Power3, OUTPUT);   // "Power3" is an output
  pinMode(LBut1.LEDPin, OUTPUT);     // "LED1" is an output
  pinMode(LBut2.LEDPin, OUTPUT);     // "LED2" is an output
  pinMode(LBut3.LEDPin, OUTPUT);     // "LED3" is an output
  pinMode(LBut4.LEDPin, OUTPUT);     // "LED4" is an output
  pinMode(LBut5.LEDPin, OUTPUT);     // "LED5" is an output

}

void loop() {

  TrackButton(&LBut1, 5);  //Track Light up button "LBut1", create 4 states

  if (LBut1.State == 0) {  // What to do on state '0'
    Blink(&LBut1, 50);     // Blinks LED at speed of 50
    ReportStateChange(&LBut1, 41);
  }
  if (LBut1.State == 1) {  // What to do on state '1'
    FadeInOut(&LBut1, 50);       // Fades LED In and Out at speed of 50
    ReportStateChange(&LBut1, 42);
  }
  if (LBut1.State == 2) {
    FreezeState(&LBut1, 100);
    Blink(&LBut1, 50);
    ReportStateChange(&LBut1, 43);
  }
  if (LBut1.State == 3) {
    FadeAsc(&LBut1, 50);
    ReportStateChange(&LBut1, 44);
  }
  if (LBut1.State == 4) {
    FadeDes(&LBut1, 50);
    ReportStateChange(&LBut1, 45);
  }

}




// NOTES, UNUSED CODE, EXTRA STUFF, ETC. //
/*
// DataP verify working block

  digitalWrite(Power1, HIGH);      // Turn off Data1
  delay(2000);                 // Wait 2 seconds
  digitalWrite(Power1, LOW);       // Turn on Data1
  digitalWrite(Power2, HIGH);      // Turn off Data2
  delay(2000);                 // Wait 2 seconds
  digitalWrite(Power2, LOW);       // Turn on Data2
  digitalWrite(Power3, HIGH);      // Turn off Data3
  delay(2000);                 // Wait 2 seconds
  digitalWrite(Power3, LOW);       // Turn on Data3

// Ultra read block

  Serial.println(hc.dist(0));
  delay(300);


// PIR read block

if (digitalRead(PIR1) == LOW) {     // Detect initial change in PIR state
  if (PIR1DetectionState == LOW) {  // No event if PIR1DetectionState is already high
    PIR1DetectionState = HIGH;      // Set PIR1DetectionState high to indicate detection
    Serial.println("Detected");     // Print confirmation message
    }
  } else {
    PIR1DetectionState = LOW;       // Set PIR1DetectionState low if PIR already detected
  }

*/
