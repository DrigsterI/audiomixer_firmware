#include "Slider.h"

// PID feedback for smooth control
float kp = 1;       // Proportional gain
float ki = 0.3;       // Integral gain
float kd = 0.000015;       // Derivative gain

Slider::Slider(int in1, int in2, int enb, int pos_pot) {
  this->in1 = in1;
  this->in2 = in2;
  this->enb = enb;
  this->pos_pot = pos_pot;
  this->lastError = 0;
  this->integralError = 0;
  this->currentPosition = 0;
  this->targetPosition = 0;

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(pos_pot, INPUT);
}


void Slider::setTarget(int newTargetPosition){
  this->targetPosition = newTargetPosition;
  Serial.print(this->targetPosition);
}

void Slider::setVolume(int newVolume) {
  int pos = map(newVolume, 0, 100, 0, 4095);
  setTarget(pos);
}

int Slider::getVolume(){
  int volume = map(this->targetPosition, 0, 4095, 0, 100);
  return volume;
}

void Slider::tick() {
  int pos = analogRead(pos_pot);
  if (abs(this->currentPosition - pos) >= 5){
    this->currentPosition = pos;
  }
  int error = this->targetPosition - this->currentPosition;
  
  // Deadband to stop near target
  if (abs(error) < 20) {
    stop();
    return;
  }
  
  // PID calculation
  this->integralError += error;
  this->integralError = constrain(this->integralError, -255, 255);
  int derivative = error - this->lastError;
  
  int pwmOutput = (int)(kp * error + ki * this->integralError + kd * derivative);
  pwmOutput = constrain(pwmOutput, -255, 255);
  
  this->lastError = error;
  
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
  this->integralError = 0;
}
