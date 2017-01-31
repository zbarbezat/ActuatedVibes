#include <MIDI.h>
#include "BED.h"
#include "midi.h"

void bed_user_input() {
  int target_angle = Serial.parseInt();
  if (target_angle != 0)
    bed_step_to_angle(target_angle);
}

void setup() {

  // Stepper
  bed_setup();

  // MIDI in
  pinMode(PIN_MIDI_LED, OUTPUT);
  midi_in.setHandleNoteOn(handle_note_on);
  midi_in.setHandleNoteOff(handle_note_off);
  midi_in.setHandleControlChange(handle_control_change);
  midi_in.begin();

  // MIDI out
  midi_out->begin(midi_out_baud);

  // Serial monitor (debug)
  Serial.begin(midi_out_baud);

  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void loop() {
  if (Serial.available())
    bed_user_input();
  midi_in.read();
}
