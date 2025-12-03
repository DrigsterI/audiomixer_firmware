#include "Slider.h"

// PID feedback for smooth control
float kp = 1;       // Proportional gain
float ki = 0.3;       // Integral gain
float kd = 0.000015;       // Derivative gain
int lastError = 0;
int integralError = 0;

Slider::Slider(int in1, int in2, int enb, int pos_pot) {
  this->in1 = in1;
  this->in2 = in2;
  this->enb = enb;
  this->pos_pot = pos_pot;

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(pos_pot, INPUT);
}

int currentPosition = 0;
int targetPosition = 0;

void Slider::setTarget(int newTargetPosition){
  targetPosition = newTargetPosition;
}

void Slider::tick() {
  int pos = analogRead(pos_pot);
  if (abs(currentPosition - pos) >= 5){
    currentPosition = pos;
  }
  int error = targetPosition - currentPosition;
  
  // Deadband to stop near target
  if (abs(error) < 20) {
    stop();
    return;
  }
  
  // PID calculation
  integralError += error;
  integralError = constrain(integralError, -255, 255);
  int derivative = error - lastError;
  
  int pwmOutput = (int)(kp * error + ki * integralError + kd * derivative);
  pwmOutput = constrain(pwmOutput, -255, 255);
  
  lastError = error;
  
  // Apply minimum PWM threshold (adjust 90 if needd)
  if (pwmOutput > 0 && pwmOutput < 90) pwmOutput = 90;
  if (pwmOutput < 0 && pwmOutput > -90) pwmOutput = -90;
  
  // Apply direction and speed
  if (pwmOutput > 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(enb, pwmOutput);
  } else if (pwmOutput < 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(enb, abs(pwmOutput));
  } else {
    stop();
  }
}

void Slider::stop() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  analogWrite(enb, 0);
  integralError = 0;
}