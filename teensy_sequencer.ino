/**
 * Copyright(c) 2015 Trevor R.H. Clarke <retrev@csh.rit.edu>
 * Subject to the GNU GPL v3 or later license.
 * See the LICENSE file or gnu.org for full license details.
 */
 
#include <Keypad.h>

// Button matrix info
// Define the array size and character mappings below.
// I used characters printed on the physical keys to make debugging easier.
// If your array isn't 4x4 you'll need to modify the ProcessKeys function to select the correct locations.
const byte ROWS = 4;
const byte COLS = 4;
// 'A' and 'F' are actually out of order for the current system, it's handled here
const char keys[ROWS][COLS] = {
  {'7','8','9','A'},
  {'4','5','6','U'},
  {'1','2','3','F'},
  {'C','0','.','E'}
};
// define the row and column mappings to the teensy digtial I/O pins
byte rowPins[ROWS] = {8,7,6,9};
byte colPins[COLS] = {20,21,22,23};

Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//////

byte sequence[16][128]; // store the sequence for each midi note
int pos = 0; // current sequence position

const int channel = 1; // the MIDI channel to send on, all MIDI channels are used for reading
int note = 36; // the current note being edited
int accentMult = 1.4;
int vel_cc = 7; // the CC used to set base velocity

// this stores the base velocity for each MIDI note
byte velocities[128];

// set the default base velocities, configure the LED for output and setup MIDI callbacks
void setup() {
  for (int i = 0; i < 128; i++) velocities[i] = 64;
  pinMode(13, OUTPUT);
  usbMIDI.setHandleRealTimeSystem(RealTimeSystem);
  usbMIDI.setHandleNoteOn(OnNoteOn);
  usbMIDI.setHandleControlChange(OnControlChange);
}

// main event loop
void loop() {
  ProcessKeys();
  usbMIDI.read();
}

// set the active note for editing
void OnNoteOn(byte channel, byte n, byte velocity) {
  note = n;
}

// adjust the base velocity for the currently edited note
// this defines the base for all positions for the current note
void OnControlChange(byte channel, byte control, byte value) {
  if (control == vel_cc) {
    velocities[note] = value;
  }
}

void ProcessKeys() {
  // process changes to the sequence for the currently edited note
  char key = kpd.getKey();
  int idx = -1;
  switch (key) {
  case '7':
    idx = 0;
    break;
  case '8':
    idx = 1;
    break;
  case '9':
    idx = 2;
    break;
  case 'F':
    idx = 3;
    break;
  case '4':
    idx = 4;
    break;
  case '5':
    idx = 5;
    break;
  case '6':
    idx = 6;
    break;
  case 'U':
    idx = 7;
    break;
  case '1':
    idx = 8;
    break;
  case '2':
    idx = 9;
    break;
  case '3':
    idx = 10;
    break;
  case 'A':
    idx = 11;
    break;
  case 'C':
    idx = 12;
    break;
  case '0':
    idx = 13;
    break;
  case '.':
    idx = 14;
    break;
  case 'E':
    idx = 15;
    break;
  }
  // if we've selected a valid key advance the sequence for that position
  // it's a tristate right now
  if (idx >= 0) {
    sequence[idx][note] = (sequence[idx][note] + 1) % 3;
  }
}

// advance the sequence and play all notes for the current sequence
// send a note off before a note on so that we don't overlap notes
void SendNextInSequence() {
  for (int idx = 0; idx < 128; idx++) {
    usbMIDI.sendNoteOff(idx, 0, channel);
    if (sequence[pos][idx] > 0) {
      byte vel = velocities[idx];
      if (sequence[pos][idx] >= 2) {
        int tmp = vel * accentMult;
        if (tmp > 127) tmp = 127;
        vel = (byte)tmp;
      }
      usbMIDI.sendNoteOn(idx, vel, channel);
    }
  }
  pos = (pos + 1) % 16;
}

// send a note off for all notes
void ClearNote() {
  for (int idx = 0; idx < 128; idx++) {
    usbMIDI.sendNoteOff(idx, 0, channel);
  }
}

// reset to the beginning of the sequence
void ResetSequence() {
  pos = 0;
}

// process MIDI real time messages (MIDI system clock)
byte counter; 
byte CLOCK = 248; 
byte START = 250; 
byte CONTINUE = 251; 
byte STOP = 252; 

void RealTimeSystem(byte realtimebyte) { 
  if (realtimebyte == 248) { // clock signal
    counter++;
    boolean advance = (counter % 6) == 0;
    if (counter % 12 == 0) { // eighth that isn't on a quarter, led off
      digitalWrite(13, LOW);
    }
    if (counter % 24 == 0) { // quarter note, turn the led on
      counter = 0;
      digitalWrite(13, HIGH);
    }
    if (advance) { // sixteenth note, next in sequence
      SendNextInSequence();
    }
  }
  if (realtimebyte == START || realtimebyte == CONTINUE) {
    counter = 0;
    if (realtimebyte == START) ResetSequence();
    digitalWrite(13, HIGH); // make sure it blinks the first time
    SendNextInSequence(); // make sure we send the next item
  } else if (realtimebyte == STOP) {
    digitalWrite(13, LOW);
    ClearNote();
  }
}
