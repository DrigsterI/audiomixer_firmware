
#include "Command.h"
#include "Config.h"
#include <ArduinoJson.h>

extern Config config;

// Definitions for static variables
std::queue<Command> commandQueue;
uint8_t pendingVolume[MAX_CHANNELS];
bool volumePending[MAX_CHANNELS] = {false};
unsigned long lastCommandTick = 0;
const unsigned long COMMAND_TICK_INTERVAL_MS = 5;

void queueVolumeCommand(uint8_t channel, uint8_t volume) {
    if (channel >= 1 && channel <= MAX_CHANNELS) {
        pendingVolume[channel - 1] = volume;
        volumePending[channel - 1] = true;
    }
}

void queueCommand(OutCommand command, uint8_t channel, const uint8_t* data, size_t len) {
    if (command == SEND_VOLUME && len >= 1) {
        queueVolumeCommand(channel, data[0]);
        return;
    }
    Command cmd;
    cmd.command = command;
    cmd.channel = channel;
    cmd.data.assign(data, data + len);
    commandQueue.push(cmd);
}

void queueCommand(OutCommand command, uint8_t channel, uint8_t value) {
    queueCommand(command, channel, &value, 1);
}

bool sendCommandNow(OutCommand command, uint8_t channel, const uint8_t* data, size_t len) {
    // Non-blocking chunked send with static state
    static uint8_t* buffer = nullptr;
    static size_t totalLen = 0;
    static size_t sent = 0;
    static bool sending = false;

    if (!sending) {
        // New command, prepare buffer
        Serial.print("Sending command: 0x");
        Serial.print((uint8_t)command, HEX);
        Serial.print(" channel: ");
        Serial.print(channel);
        Serial.print(" data:");
        for (size_t i = 0; i < len; ++i) {
            Serial.print(" ");
            Serial.print(data[i]);
        }
        Serial.println();
        totalLen = 2 + len + 1;
        buffer = new uint8_t[totalLen];
        buffer[0] = command;
        buffer[1] = channel;
        memcpy(buffer + 2, data, len);
        buffer[2 + len] = EOF;
        sent = 0;
        sending = true;
    }

    int avail = USBSerial.availableForWrite();
    if (avail <= 0) {
        // No space, try again next tick
        return false;
    }
    // For all but the last chunk, do not send EOF
    size_t toSend = totalLen - sent;
    size_t chunkLen = toSend;
    if (toSend > 8) chunkLen = (avail < 8) ? avail : 8;
    if (sent + chunkLen >= totalLen) {
        // Last chunk: include EOF
        chunkLen = totalLen - sent;
    } else {
        // Not last chunk: do not send EOF
        if (sent + chunkLen > totalLen - 1) chunkLen = totalLen - 1 - sent;
    }
    size_t written = USBSerial.write(buffer + sent, chunkLen);
    sent += written;
    if (sent >= totalLen) {
        // Done
        delete[] buffer;
        buffer = nullptr;
        totalLen = 0;
        sent = 0;
        sending = false;
        return true;
    }
    // Not done yet, try again next tick
    return false;
}

void commandTick() {
    unsigned long now = millis();
    if (now - lastCommandTick >= COMMAND_TICK_INTERVAL_MS) {
        lastCommandTick = now;
        if (!USBSerial) {
            // USB serial not connected, skip sending
            return;
        }
        if (!commandQueue.empty()) {
            Command& cmd = commandQueue.front();
            int totalLen = 2 + cmd.data.size() + 1; // header + data + EOF
            bool done = sendCommandNow(cmd.command, cmd.channel, cmd.data.data(), cmd.data.size());
            if (done) {
                commandQueue.pop();
            }
            // Always return after one send attempt to keep loop non-blocking
            return;
        }
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (volumePending[i]) {
                int totalLen = 2 + 1 + 1; // header + 1 byte data + EOF
                bool done = sendCommandNow(SEND_VOLUME, i + 1, &pendingVolume[i], 1);
                if (done) {
                    volumePending[i] = false;
                }
                // Always return after one send attempt to keep loop non-blocking
                return;
            }
        }
    }
}

void recieveCommand(uint8_t *buf, size_t len) {
  enum InCommand command = (enum InCommand)buf[0];
  uint8_t channel = buf[1];
  
  uint8_t *value_start = &buf[2];
  size_t value_len = len - 2;

  switch (command) {
    case REQUEST_INFO: {
      // Prepare slider info as JSON
      extern Config config;
      StaticJsonDocument<512> doc;
      JsonArray sliders_array = doc.createNestedArray("sliders");
      const std::vector<SliderInfo>& sliders_info = config.getSlidersInfo();
      for (const auto& slider : sliders_info) {
        JsonObject obj = sliders_array.createNestedObject();
        obj["name"] = slider.name.c_str();
        obj["set_volume_action"] = slider.set_volume_action.c_str();
      }
      String serialized;
      serializeJson(doc, serialized);
      queueCommand(SEND_INFO, channel, (const uint8_t*)serialized.c_str(), serialized.length());
      Serial.println("Handled REQUEST_INFO, queued SEND_INFO");
      break;
    }
    case SET_VOLUME: {
      uint8_t value = value_start[0];
      Serial.print("Received SET_VOLUME for channel ");
      Serial.print(channel);
      Serial.print(": ");
      Serial.println(value);
      // Here you could update a slider, etc.
      break;
    }
    default:
      Serial.print("Unknown command: 0x");
      Serial.println((uint8_t)command, HEX);
      break;
  }
}
