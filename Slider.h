#ifndef SLIDER_H
#define SLIDER_H

#include <Arduino.h>

typedef void (*CallbackFunction)(int);

class Slider {
  private:
    int in1;
    int in2;
    int enb;
    int pos_pot;
    int currentPosition = 0;
    int targetPosition = 0;
    int lastError = 0;
    int integralError = 0;
    bool targetReached = true;
    CallbackFunction callback;
  public:
    Slider(int in1, int in2, int enb, int pos_pot, CallbackFunction callback);
    void setTarget(int targetPosition);
    void tick();
    void stop();
    void setVolume(int newVolume);
    int getVolume();
};

#endif
