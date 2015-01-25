# Teensy Sequencer
A simple MIDI sequencer for a 4x4 button array and a Teensy 3.1 microcontroller.

This arduino sketch works with Teensy only due to the Teensy specific USB MIDI library.

## Hardware
This uses a simple 4x4 matrix button array and the arduino Keypad library for input. No external power is necessary.
The array pins are defined in the sketch.

## Software
The software is pretty basic at this point. Each button corresponds to a place in a 16 step sequence. The default is to use 16th notes defining a single bar sequence.
Pressing a key cycles that position's note. It's a tristate with off, on, and accent. Accent is defined as 140% of the base velocity (clamped to max).

The actively edited note is set by sending a note on message to the sequencer.

CC messages can be sent to control other aspects of the sequencer:
 - CC7 sets the base velocity for the active note
