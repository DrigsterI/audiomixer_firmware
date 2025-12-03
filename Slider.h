#ifndef SLIDER_H
#define SLIDER_H

#include <Arduino.h>

class Slider {
  private:
    int in1;
    int in2;
    int enb;
    int pos_pot;
  public:
    Slider(int in1, int in2, int enb, int pos_pot);
    void setTarget(int targetPosition);
    void tick();
    void stop();
};

#endif