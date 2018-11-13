#include "DualMC33926MotorShield.h"
#include "PinChangeInt.h"
#include "math.h"

#define PinMotor1Sensor1 2
#define PinMotor1Sensor2 3
#define PinMotor2Sensor1 5
#define PinMotor2Sensor2 6

// Initialize:

// Constants:
double K = 1.0;
double B = 1.0;
double N_enc = 32.0; // Segments on Encoder = 32
double dia = 0.071; // Dia = 71 mm
double WB = 0.157;  // Wheel Base = 157 mm
double w = 10.0; // Weighting factor
int flag = 0;

// Parameters:
double PWM_ref = 200;
double PWM_l = PWM_ref;
double PWM_r = PWM_ref;

// Variables:
double right_count = 0.0;
double left_count = 0.0;
double err_count = 0.0;
double n_l_ref = 0.0, n_r_ref = 0.0;
double del_x, x, y, theta = 0.0;
double S_l = 0.0, S_r = 0.0;
double loc[3];
double S_l_ref, S_r_ref;

DualMC33926MotorShield md;

void setup()
{
  Serial.begin(115200);
  md.init();
  //Attach Interrupts
  attachPinChangeInterrupt(PinMotor1Sensor1, right_encoder_isr, CHANGE);
  attachPinChangeInterrupt(PinMotor1Sensor2, right_encoder_isr, CHANGE);

  attachPinChangeInterrupt(PinMotor2Sensor1, left_encoder_isr, CHANGE);
  attachPinChangeInterrupt(PinMotor2Sensor2, left_encoder_isr, CHANGE);
}

void loop()
{
  left_encoder_isr();
  right_encoder_isr();
  first_stretch();
  //motor_control();
  //Serial.println("Right:" + String(left_count) + " Left:" + String(right_count) + "; S_l=" + String(S_l) + " S_r=" + String(S_r) + "; theta:" + String(theta * (180.0 / M_PI)));
  //Serial.println(Etime);
  delay(1000);
  Serial.print("S_l:");
  Serial.print(S_l);
  Serial.print(" S_r:");
  Serial.print(S_r);
  Serial.print("n_l:");
  Serial.print(left_count);
  Serial.print("n_r:");
  Serial.println(right_count);
}

void right_encoder_isr() {
  static int8_t lookup_table_r[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
  static uint8_t enc_val_r = 0;
  enc_val_r = enc_val_r << 2;
  enc_val_r = enc_val_r | ((PIND & 0b00001100) >> 2);
  right_count = right_count + lookup_table_r[enc_val_r & 0b1111];
}

void left_encoder_isr() {
  static int8_t lookup_table_l[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
  static uint8_t enc_val_l = 0;
  enc_val_l = enc_val_l << 2;
  enc_val_l = enc_val_l | ((PIND & 0b01100000) >> 5);
  //    Serial.println(enc_val_l);
  left_count = left_count + lookup_table_l[enc_val_l & 0b1111];
}

void get_location() {
  S_l = ((M_PI * dia * left_count) / N_enc);
  S_r = ((M_PI * dia * right_count) / N_enc);
  theta = atan2(((S_r - S_l) / 2.0), (WB / 2.0));
  del_x = (S_l + S_r) / 2;
  x = del_x * cos(theta);
  y = del_x * sin(theta);
  loc[0] = x;
  loc[1] = y;
  loc[2] = theta;
}

void set_refcounts() {
  n_l_ref = (N_enc * S_l_ref) / (M_PI * dia);
  n_r_ref = (N_enc * S_r_ref) / (M_PI * dia);
}

void motor_control() {
  get_location();
  if ((S_l < S_l_ref) && (S_r < S_r_ref)) {
    throttle();
  }
  else {
    brake();
    S_l_ref = 0;
    S_r_ref = 0;
  }
}

void throttle() {
  md.setM1Speed(PWM_l); //Left
  md.setM2Speed(PWM_r); //Right
}

void brake() {
  md.setM1Speed(0); //Left
  md.setM2Speed(0); //Right
}

void first_stretch() {
  PWM_l = 200;
  PWM_r = 200;
  S_l_ref = 0.6096;
  S_r_ref = 0.6096;
  /*
    if (left_count > right_count) {
    PWM_l = PWM_l - w * (err_count);
    }
    else if (right_count > left_count) {
    PWM_r = PWM_r - w * (err_count);
    }
  */
  motor_control();
  flag = 1;
}

void curve_path() {
  PWM_l = 100;
  PWM_r = 200;
  S_l_ref = 0.83;
  S_r_ref = 0.57;
  /*
    if (left_count > right_count) {
    PWM_l = PWM_l - w * (err_count);
    }
    else if (right_count > left_count) {
    PWM_r = PWM_r - w * (err_count);
    }
  */
  motor_control();
  flag = 2;
}

void second_stretch() {
  S_l_ref = 2 * 0.6096;
  S_r_ref = 2 * 0.6096;
  get_location();
  set_refcounts();
  /*
    if (left_count > right_count) {
    PWM_l = PWM_l - w * (err_count);
    }
    else if (right_count > left_count) {
    PWM_r = PWM_r - w * (err_count);
    }
  */
  motor_control();
}