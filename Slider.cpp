#include "Slider.h"

// PID feedback for smooth control
float kp = 1;       // Proportional gain
float ki = 0.3;       // Integral gain
float kd = 0.000015;       // Derivative gain

Slider::Slider(int in1, int in2, int enb, int pos_pot, CallbackFunction callback) {
  this->in1 = in1;
  this->in2 = in2;
  this->enb = enb;
  this->pos_pot = pos_pot;
  this->callback = callback;

  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enb, OUTPUT);
  pinMode(pos_pot, INPUT);
}

void Slider::setTarget(int newTargetPosition){
  targetPosition = newTargetPosition;
  targetReached = false;
}

void Slider::setVolume(int newVolume) {
  Serial.printf("setVolume: %d\n", newVolume);
  int pos = map(newVolume, 0, 100, 0, 4095);
  setTarget(pos);
}

int Slider::getVolume(){
  int volume = map(currentPosition, 0, 4095, 0, 100);
  return volume;
}

void Slider::tick() {
  int pos = analogRead(pos_pot);
  if (abs(currentPosition - pos) >= 120){
    if (targetReached) {
      callback(getVolume());
      targetPosition = pos;
    }
    currentPosition = pos;
  }

  int error = targetPosition - currentPosition;

  // Deadband to stop near target
  if (abs(error) < 20 && targetReached == false) {
    stop();
    targetReached = true;
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
