#ifndef COMMAND_H
#define COMMAND_H

#include <Arduino.h>
#include <queue>
#include "USB.h"

enum InCommand {
  REQUEST_INFO = 0x01,
  SET_VOLUME = 0x02,
};

enum OutCommand {
  SEND_INFO = 0x81,
  SEND_VOLUME = 0x82,
};


// Command structure for queue
struct Command {
  OutCommand command;
  uint8_t channel;
  std::vector<uint8_t> data;
};

#define MAX_CHANNELS 8

extern std::queue<Command> commandQueue;
extern uint8_t pendingVolume[MAX_CHANNELS];
extern bool volumePending[MAX_CHANNELS];
extern USBCDC USBSerial;
extern unsigned long lastCommandTick;
extern const unsigned long COMMAND_TICK_INTERVAL_MS;

void queueVolumeCommand(uint8_t channel, uint8_t volume);
void queueCommand(OutCommand command, uint8_t channel, const uint8_t* data, size_t len);
void queueCommand(OutCommand command, uint8_t channel, uint8_t value);
bool sendCommandNow(OutCommand command, uint8_t channel, const uint8_t* data, size_t len);
void commandTick();
void recieveCommand(uint8_t *buf, size_t len);

#endif
