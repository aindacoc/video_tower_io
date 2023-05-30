/*
  Program title: Video Wall I/O
  Description: Handles PIR, light up button, and potentiometer I/O and controls Dataprobe K415 Power Strip
  Created by: Andres Indacochea
  Created on: 4/26/23
  Last Modified by: Andres Indacochea
  Last Modified on: 5/25/23
*/

// Create structure for Light up Buttons
struct LBut {               // Name of structure is "LBut"
  const int ButPin;         // "LBut" has an associated pin for button input
  const byte LEDPin;        // "LBut" has an assocaited pin for LED output
  int BlinkTrack;           // Variable to track cycles for blinking
  unsigned int FadeTrack;   // Variable to track cylces and set LED brightness for fading
  unsigned int FreezeTrack; // Variable to track cycles for freezing button state
  unsigned int CycleTrack;  // Variable to track cycles for "time" commands
  bool TrackDirection;      // Variable to switch direction of tracks
  bool ButtonPress;         // Variable to detect and hold button presses
  bool StateChange;         // Variable to signal the change in state
  bool Frozen;              // Variable to store freeze state of button
  bool SkipMe;              // Variable to skip other lighting routines during freezing
  bool Debounce;            // Variable to hold button input for debounce
  byte DebounceTrack;       // Variable to track the number of cycles to debounce for
  byte State;               // Variable to hold the state of the button
  byte NumberOfStates;      // Variable to hold the number of states
};

struct PIR {
  const int Pin;
  unsigned int CycleTrack;
  bool SerialSent;
  bool Advanced;
  bool Powered;
};

// Setup Hardware Pins, DO NOT CHANGE UNLESS HARDWARE CHANGES ARE MADE
const int Pot2 = A6;        // "Pot2" connected to pin A6
LBut LBut1 = {A0, 5};       // "LBut1" connected to pin A0, LED connected to pin D5
LBut LBut2 = {A1, 6};       // "LBut2" connected to pin A1, LED connected to pin D6
LBut LBut3 = {2, 9};        // "LBut3" connected to pin D2, LED connected to pin D9
LBut LBut4 = {3, 10};       // "LBut4" connected to pin D3, LED connected to pin D10
LBut LBut5 = {4, 11};       // "LBut5" connected to pin D4, LED connected to pin D11
PIR PIR2 = {7};             // "PIR2" connected to pin D7
const byte Power1 = 8;      // "Power1" connected to pin D8
const byte Power2 = 12;     // "Power2" connected to pin D12
const byte Power3 = 13;     // "Power3" connected to pin D13

//Global variables used by non-structured functions
bool SwitchTrack;

//Begin cycle-friendly functions, SEE BOTTOM FOR FUNCTION DESCRIPTIONS
void TrackButton (LBut *ofInt, byte NumberOfStates) {
  ofInt->NumberOfStates = NumberOfStates;
  if (digitalRead(ofInt->ButPin) == LOW) {
    if (ofInt->ButtonPress == LOW && ofInt->Debounce == LOW) {
      ofInt->ButtonPress = HIGH;
      ofInt->StateChange = HIGH;
      ofInt->Frozen = HIGH;
      ofInt->Debounce = HIGH;
      SwitchTrack = HIGH;
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


void FreezeState (LBut *ofInt, unsigned int FreezeTime) {
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


void CycleAdvanceState (LBut *ofInt, unsigned int AdvanceCycles) {
  if (ofInt->SkipMe == LOW) {
    ofInt->CycleTrack = ofInt->CycleTrack + 1;
    if (ofInt->CycleTrack >= AdvanceCycles) {
      ofInt->CycleTrack = 0;
      ofInt->StateChange = HIGH;
      ofInt->State = ofInt->State + 1;
      if (ofInt->State >= ofInt->NumberOfStates) {
        ofInt->State = 0;
      }
    }
  }
}


void ReportStateChange (LBut *ofInt, unsigned int MessageToSend) {
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
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack) / 253));
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
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack) / 255));
    delay(1);
  }
}

void FadeDes (LBut *ofInt, byte FadeSpeed) {
  if (ofInt->SkipMe == LOW) {
    ofInt->FadeTrack = ofInt->FadeTrack - FadeSpeed;
    analogWrite(ofInt->LEDPin, byte((ofInt->FadeTrack) / 255));
    delay(1);
  }
}

void SwitchOn (byte PowerControl) {
  if (SwitchTrack == HIGH) {
    digitalWrite(PowerControl, LOW);
    SwitchTrack = LOW;
  }
}

void SwitchOff (byte PowerControl) {
  if (SwitchTrack == HIGH) {
    digitalWrite(PowerControl, HIGH);
    SwitchTrack = LOW;
  }
}

void SerialOnPIR (PIR *ofInt, byte MessageToSend) {
  if (digitalRead(ofInt->Pin) == LOW) {     // Detect initial change in PIR status
    if (ofInt->SerialSent == LOW) {                // No event if State is already high
      ofInt->SerialSent = HIGH;                    // Set State high to indicate detection
      Serial.print(MessageToSend);            // Print confirmation message
    }
  } else {
    ofInt->SerialSent = LOW;                     // Set State low if PIR already detected
  }
}

void AdvanceOnPIR (LBut *LButOI, PIR *PIROI) {
  if (LButOI->SkipMe == LOW) {
    if (digitalRead(PIROI->Pin) == LOW) {
      if (PIROI->Advanced == LOW) {
        PIROI->Advanced = HIGH;
        LButOI->StateChange = HIGH;
        LButOI->State = LButOI->State + 1;
        if (LButOI->State >= LButOI->NumberOfStates) {
          LButOI->State = 0;
        }
      } else {
        PIROI->Advanced = LOW;
      }
    }
  }
}

void CycledPowerOffPIR (byte PowerControl, PIR *PIROI, unsigned int PowerOffCycles) {
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

void CycledPowerOnPIR (byte PowerControl, PIR *PIROI, unsigned int PowerOnCycles) {
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

  Serial.begin(9600);        // Begin serial communication at 9600 baud rate

  // Declare pins as inputs or outputs
  pinMode(Pot2, INPUT);      // "Pot2" is an input
  pinMode(PIR2.Pin, INPUT);      // "PIR2" is an input
  pinMode(LBut1.ButPin, INPUT);     // "LBut1" is an input
  pinMode(LBut2.ButPin, INPUT);     // "LBut2" is an input
  pinMode(LBut3.ButPin, INPUT);     // "LBut3" is an input
  pinMode(LBut4.ButPin, INPUT);     // "LBut4" is an input
  pinMode(LBut5.ButPin, INPUT);     // "LBut5" is an input
  pinMode(Power1, OUTPUT);   // "Power1" is an output
  pinMode(Power2, OUTPUT);   // "Power2" is an output
  pinMode(Power3, OUTPUT);   // "Power3" is an output
  pinMode(LBut1.LEDPin, OUTPUT);     // LED of "LBut1" is an output
  pinMode(LBut2.LEDPin, OUTPUT);     // LED of "LBut2" is an output
  pinMode(LBut3.LEDPin, OUTPUT);     // LED of "LBut3" is an output
  pinMode(LBut4.LEDPin, OUTPUT);     // LED of "LBut4" is an output
  pinMode(LBut5.LEDPin, OUTPUT);     // LED of "LBut5" is an output

}

void loop() {
  TrackButton(&LBut1, 2);

  switch (LBut1.State) {
    case 0:
      Blink(&LBut1, 100);
      ReportStateChange(&LBut1, 258);
      break;
    case 1:
      FadeDes(&LBut1, 100);
      break;
  }
}


/*------- CYCLE FRIENDLY FUNCTION LIST --------//

  void TrackButton (&LBut, byte NumberOfStates)
      This function will track a button's state from 0,1 ... NumberOfStates and allow you to select
      other functions condtionally by the state of the button. Recommended using switch...case
      statements to define button states. Use LBut.State as defining variable.

  void FreezeState (&LBut, unsigned int FreezeCycles)
      This function freezes the state and will indicate with a fast blink. As a result, it
      skips other LED and state advance functions while active.

  void CycleAdvanceState (&LBut, unsigned int AdvanceCycles)
      This function advances the button state after a number of cycles. Skipped by freezing.

  void ReportStateChange (&LBut, byte MessageToSend)
      This function will see a change in state and send a serial message.

  void LEDOn (&LBut)
      This function turns the LED on. Must not use with other lighting routines in the same state.

  void LEDOff (&LBut)
      This function turns the LED on.

  void Blink (&LBut, byte BlinkSpeed)
      This function blinks the LED.

  void FadeInOut (&LBut, byte FadeSpeed)
      This function fades the LED in and out.

  void FadeAsc (&LBut, byte FadeSpeed)
      This function does an ascending fade on the LED then starts over.

  void FadeDes (&LBut, byte FadeSpeed)
      This function does an descending fade on the LED then starts over.

  void SwitchOn (byte PowerControl)
      This function turns on a power control once per state.

  void SwitchOff (byte PowerControl)
      This function turns off a power control once per state.

  void SerialOnPIR (&PIR, byte MessageToSend)
      This function sends a serial message when a PIR event is detected.

  void AdvanceOnPIR (&LBut, &PIR)
      This function advances the button state when a PIR event is detected.

  void CycledPowerOffPIR (byte PowerControl, &PIR, unsigned int PowerOnCycles)
      This function turns off a power control for a designated number of cycles when a PIR event is detected.

  void CycledPowerOnPIR (byte PowerControl, &PIR, unsigned int PowerOnCycles)
      This function turns on a power control for a designated number of cycles when a PIR event is detected.
*/
