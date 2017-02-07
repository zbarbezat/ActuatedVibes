//#define MIDI_SERIAL_DEBUG

const int PIN_MIDI_LED = 5;
int previousVal = 0;

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, midi_in);   // MIDI (serial) in on hardware serial 2
HardwareSerial *midi_out = &Serial3;        // Simplified MIDI-like serial out on hardware serial 3
const int midi_out_baud = 9600;

void handle_note_on(byte ch, byte note, byte vel) {
  midi_out->write(note);
  midi_out->write(vel);
  analogWrite(PIN_MIDI_LED, vel);

#ifdef MIDI_SERIAL_DEBUG
  Serial.print(note);
  Serial.print(" ");
  Serial.println(vel);
#endif
}

void handle_note_off(byte ch, byte note, byte vel) {
  midi_out->write(note);
  midi_out->write(0);
  analogWrite(PIN_MIDI_LED, 0);

#ifdef MIDI_SERIAL_DEBUG
  Serial.print(note);
  Serial.print(" ");
  Serial.println(0);
#endif
}

void handle_control_change(byte ch, byte num, byte val) {
  //  if (num == 64) {      // Damper pedal
  if (val > 63) {   // On
    previousVal = val;
    if (previousVal < 63) {
      bed_step_to_angle(bed_sustain_up_angle);
    }
  }
  if (val < 63) {            // Off
    previousVal = val;
    if (previousVal > 64) {
      bed_step_to_angle(bed_sustain_down_angle);
    }
  }
}


