#include <SoftwareSerial.h>

const int baudrate = 9600;

unsigned long current_time = 0;
unsigned long note_1_on_time = 0;
unsigned long note_2_on_time = 0;
const unsigned long note_time_ms = 30;

bool note_1_active = false;
bool note_2_active = false;

// [Gb3, Ab3], [Bb3, Db4], [Eb4, Gb 4], [Ab4, Bb4], [Db5, Eb5], [Gb5, Ab5], [Bb5, Db6], [Eb6, xxx]
// [ 42,  44], [ 46,  49], [ 51,  54], [ 56,  58], [ 61,  63], [ 66,  68], [ 70,  73], [ 75, xxx]

// [F3, G3], [A3, B3], [C4, D4], [E4, F4], [G4, A4], [B4, C5], [D5, E5], [F5, G5], [A5, B5], [C6, D6], [E6, F6]
// [41, 43], [45, 47], [48, 50], [52, 53], [55, 57], [59, 60], [62, 64], [65, 67], [69, 71], [72, 74], [76, 77]
const int note_1 = 70;
const int note_2 = 73;

const int PIN_OUT_1 = 0;
const int PIN_OUT_2 = 1;
const int PIN_TX = 4;   // Physical pin 3
const int PIN_RX = 3;   // Physical pin 2

SoftwareSerial midi_serial(PIN_RX, PIN_TX);

void setup() {
  pinMode(PIN_TX, OUTPUT);
  pinMode(PIN_RX, INPUT);
  pinMode(PIN_OUT_1, OUTPUT);
  pinMode(PIN_OUT_2, OUTPUT);
  midi_serial.begin(baudrate);
}

byte midi_command;
byte midi_note;
byte midi_velocity;
void loop() { 
  
  while (midi_serial.available() > 1) {
    midi_note = midi_serial.read();
    midi_velocity = midi_serial.read();
    if (midi_note == note_1 && midi_velocity > 0) {
      analogWrite(PIN_OUT_1, midi_velocity);
      note_1_on_time = millis();
      note_1_active = true;
    }
    if (midi_note == note_2 && midi_velocity > 0) {
      analogWrite(PIN_OUT_2, midi_velocity);
      note_2_on_time = millis();
      note_2_active = true;
    }
  }
  current_time = millis();
  if (note_1_active && (current_time - note_1_on_time) > note_time_ms) {
    analogWrite(PIN_OUT_1, 0);
    note_1_active = false;
  }
  if (note_2_active && (current_time - note_2_on_time) > note_time_ms) {
    analogWrite(PIN_OUT_2, 0);
    note_2_active = false;
  } 
}
