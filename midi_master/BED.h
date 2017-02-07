//#define BED_SERIAL_DEBUG

const int PIN_BED_DIR = 14;     // Stepper direction
const int PIN_BED_STEP = 15;    // Stepper control signal (step on falling edge)
const int PIN_BED_MS3 = 16;     // Microstep resolution LSB
const int PIN_BED_MS2 = 17;     // ...
const int PIN_BED_MS1 = 18;     // Microstep resolution MSB
const int PIN_BED_ENABLE = 19;  // Stepper driver enable
const int PIN_HOME_SWITCH = 23; // Stepper home switch input

typedef enum BED_bed_step_res {     // Step resolutions
  kBED_bed_step_res_full = 0,
  kBED_bed_step_res_half,
  kBED_bed_step_res_quarter,
  kBED_bed_step_res_eighth,
  kBED_bed_step_res_sixteenth
} BED_bed_step_res;

typedef enum BED_bed_step_dir {     // Step directions
  kBED_bed_step_dir_forward = 0,
  kBED_bed_step_dir_backward
} BED_bed_step_dir;

BED_bed_step_res bed_step_res;                // Current motor step resolution
BED_bed_step_dir bed_step_dir;                // Current motor step direction

const int bed_steps_per_rev = 200;        // Number of full steps per revolution (Wantai 57BYGH420-2)
int bed_current_angle = 0;                // Current motor angle in sixteenths of a step
int bed_sustain_down_angle = 0;  // Target motor angle (in sixteenths) for sustain bar down
int bed_sustain_up_angle = 0;
int bed_sustain_diff_angle = 0;

/* Set the BED step direction */
void bed_set_dir(BED_bed_step_dir dir) {
  bed_step_dir = dir;
  if (bed_step_dir == kBED_bed_step_dir_forward)
    digitalWrite(PIN_BED_DIR, LOW);
  else
    digitalWrite(PIN_BED_DIR, HIGH);
}

/* Set the BED step resolution using MS1, MS2, and MS3 */
void bed_set_bed_step_res(BED_bed_step_res res) {
  bed_step_res = res;
  switch (bed_step_res) {
    case kBED_bed_step_res_full:
      digitalWrite(PIN_BED_MS1, LOW);
      digitalWrite(PIN_BED_MS2, LOW);
      digitalWrite(PIN_BED_MS3, LOW);
      break;
    case kBED_bed_step_res_half:
      digitalWrite(PIN_BED_MS1, HIGH);
      digitalWrite(PIN_BED_MS2, LOW);
      digitalWrite(PIN_BED_MS3, LOW);
      break;
    case kBED_bed_step_res_quarter:
      digitalWrite(PIN_BED_MS1, LOW);
      digitalWrite(PIN_BED_MS2, HIGH);
      digitalWrite(PIN_BED_MS3, LOW);
      break;
    case kBED_bed_step_res_eighth:
      digitalWrite(PIN_BED_MS1, HIGH);
      digitalWrite(PIN_BED_MS2, HIGH);
      digitalWrite(PIN_BED_MS3, LOW);
      break;
    case kBED_bed_step_res_sixteenth:
      digitalWrite(PIN_BED_MS1, HIGH);
      digitalWrite(PIN_BED_MS2, HIGH);
      digitalWrite(PIN_BED_MS3, HIGH);
      break;
  }
}

/* Advance a number of steps in the current direction, at the current resolution */
void bed_step(int num_steps) {

  for (int i = 0; i < num_steps; i++) {   // Step (falling edge)
    digitalWrite(PIN_BED_STEP, HIGH);
    delay(1);
    digitalWrite(PIN_BED_STEP, LOW);
    delay(1);
  }

  // Keep track of current motor angle relative to home
  int num_sixteenths = num_steps;
  switch (bed_step_res) {
    case kBED_bed_step_res_full:
      num_sixteenths *= 16;
      break;
    case kBED_bed_step_res_half:
      num_sixteenths *= 8;
      break;
    case kBED_bed_step_res_quarter:
      num_sixteenths *= 4;
      break;
    case kBED_bed_step_res_eighth:
      num_sixteenths *= 2;
      break;
    case kBED_bed_step_res_sixteenth:
    default:
      break;
  }
  if (bed_step_dir == kBED_bed_step_dir_forward)
    bed_current_angle += num_sixteenths;
  else
    bed_current_angle -= num_sixteenths;
}

/* Step to the specified angle (in sixteenth steps) as quickly as possible (lowest resolution) */
void bed_step_to_angle(int target_angle) {

  /* Take modulus with number of sixteenth steps per revoluion
     so we don't make more than a full rotation */
  target_angle %= bed_steps_per_rev * 16;

  // Determine number of sixteenth steps and direction
  int num_sixteenths = target_angle - bed_current_angle;
  bed_set_dir(num_sixteenths > 0 ? kBED_bed_step_dir_forward : kBED_bed_step_dir_backward);
  num_sixteenths = abs(num_sixteenths);

  int num_full_steps = num_sixteenths / 16;

#ifdef BED_SERIAL_DEBUG
  Serial.print(" 1/1: ");
  Serial.println(num_full_steps);
#endif

  if (num_full_steps > 0) {
    bed_set_bed_step_res(kBED_bed_step_res_full);
    bed_step(num_full_steps);
    num_sixteenths -= num_full_steps * 16;
  }

  int num_half_steps = num_sixteenths / 8;

#ifdef BED_SERIAL_DEBUG
  Serial.print(" 1/2: ");
  Serial.println(num_half_steps);
#endif

  if (num_half_steps > 0) {
    bed_set_bed_step_res(kBED_bed_step_res_half);
    bed_step(num_half_steps);
    num_sixteenths -= num_half_steps * 8;
  }

  int num_quarter_steps = num_sixteenths / 4;

#ifdef BED_SERIAL_DEBUG
  Serial.print(" 1/4: ");
  Serial.println(num_quarter_steps);
#endif

  if (num_quarter_steps > 0) {
    bed_set_bed_step_res(kBED_bed_step_res_quarter);
    bed_step(num_quarter_steps);
    num_sixteenths -= num_quarter_steps * 4;
  }

  int num_eighth_steps = num_sixteenths / 2;

#ifdef BED_SERIAL_DEBUG
  Serial.print(" 1/8: ");
  Serial.println(num_eighth_steps);
#endif

  if (num_eighth_steps > 0) {
    bed_set_bed_step_res(kBED_bed_step_res_eighth);
    bed_step(num_eighth_steps);
    num_sixteenths -= num_eighth_steps * 2;
  }

#ifdef BED_SERIAL_DEBUG
  Serial.print("1/16:");
  Serial.println(num_sixteenths);
#endif

  if (num_sixteenths > 0) {
    bed_set_bed_step_res(kBED_bed_step_res_sixteenth);
    bed_step(num_sixteenths);
  }
}

/* Step forward at lowest resolution until the motor trips the home switch. */
void bed_find_home() {
  bed_set_dir(kBED_bed_step_dir_forward);
  bed_set_bed_step_res(kBED_bed_step_res_sixteenth);
  while (digitalRead(PIN_HOME_SWITCH)) {    // Step (falling edge)
    digitalWrite(PIN_BED_STEP, HIGH);
    delay(1);
    digitalWrite(PIN_BED_STEP, LOW);
    delay(1);
  }
}

/* Initiates calibration process by stepping at lowest resolution until user input. */
void bed_calib() {
  int complete = 0;
  Serial.begin(9600);
  Serial.setTimeout(1000);
  Serial.println("1 for calibration, 2 to load from EEPROM");
  char rx_byte = 0;
  while (complete == 0) {
    if (Serial.available() > 0) { // is a character available?
      rx_byte = Serial.read();    // get the character
      if (rx_byte == '1') {
        Serial.println("Enter any digit when motor reaches desired position");
        int stopup = 0;
        while (stopup == 0) {
          if (Serial.available() > 0) {
            bed_sustain_up_angle = bed_current_angle;
            EEPROM.write(0, bed_sustain_up_angle);
            Serial.println("Wrote top angle to EEPROM");
            stopup = 1;
          }
          else {
            bed_step(1);
          }
        }

        delay(3000);
        Serial.println("Now calibrating down angle, please enter 2 when motor reaches desired angle");
        bed_set_dir(kBED_bed_step_dir_backward);
        
        int stopdown = 0;
        while (stopdown == 0) {
          int downinput = Serial.read();
          if (downinput == '2') {
            bed_sustain_down_angle = bed_current_angle;
            EEPROM.write(1, bed_sustain_down_angle);
            Serial.println("Wrote bottom angle to EEPROM");
            stopdown = 1;
            bed_set_dir(kBED_bed_step_dir_forward);
            Serial.println("Please wait a few seconds before any note input");
            bed_sustain_diff_angle = bed_sustain_up_angle - bed_sustain_down_angle;
            complete = 1;
          }
          else {
            bed_step(1);
          }
        }
      }
      else if (rx_byte == '2') {
        bed_sustain_up_angle = EEPROM.read(0);
        bed_sustain_down_angle = EEPROM.read(1);
        Serial.print("Calibration angle ");
        Serial.print(bed_sustain_up_angle);
        Serial.print(" and ");
        Serial.print(bed_sustain_down_angle);
        Serial.println(" loaded from EEPROM");
        bed_sustain_diff_angle = bed_sustain_up_angle - bed_sustain_down_angle;
        complete = 1;
      }
      else {
        Serial.println("not a valid option");
      }
    }
  }
}

/* Set pin modes for BED (Big Easy Driver) control */
void bed_setup() {
  pinMode(PIN_BED_DIR, OUTPUT);
  pinMode(PIN_BED_STEP, OUTPUT);
  pinMode(PIN_BED_MS3, OUTPUT);
  pinMode(PIN_BED_MS2, OUTPUT);
  pinMode(PIN_BED_MS1, OUTPUT);
  pinMode(PIN_BED_ENABLE, OUTPUT);
  pinMode(PIN_HOME_SWITCH, INPUT);
  digitalWrite(PIN_BED_ENABLE, HIGH);
  digitalWrite(PIN_BED_ENABLE, LOW);    // Enable the motor
  bed_find_home();
  bed_calib();
  bed_set_dir(kBED_bed_step_dir_forward);
  bed_set_bed_step_res(kBED_bed_step_res_sixteenth);
  
}
