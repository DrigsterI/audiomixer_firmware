#include "USB.h"

#include "Slider.h"

Slider slider1 = Slider(2, 3, 4, A0);
Slider slider2 = Slider(5, 6, 7, A1);
const int sliderCount = 2;
Slider sliders[2] = {slider1, slider2};

USBCDC USBSerial;

static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT: Serial.println("USB PLUGGED"); break;
      case ARDUINO_USB_STOPPED_EVENT: Serial.println("USB UNPLUGGED"); break;
      case ARDUINO_USB_SUSPEND_EVENT: Serial.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en); break;
      case ARDUINO_USB_RESUME_EVENT:  Serial.println("USB RESUMED"); break;

      default: break;
    }
  } else if (event_base == ARDUINO_USB_CDC_EVENTS) {
    arduino_usb_cdc_event_data_t *data = (arduino_usb_cdc_event_data_t *)event_data;
    switch (event_id) {
      case ARDUINO_USB_CDC_CONNECTED_EVENT:    Serial.println("CDC CONNECTED"); break;
      case ARDUINO_USB_CDC_DISCONNECTED_EVENT: Serial.println("CDC DISCONNECTED"); break;
      case ARDUINO_USB_CDC_LINE_STATE_EVENT:   Serial.printf("CDC LINE STATE: dtr: %u, rts: %u\n", data->line_state.dtr, data->line_state.rts); break;
      case ARDUINO_USB_CDC_LINE_CODING_EVENT:
        Serial.printf(
          "CDC LINE CODING: bit_rate: %lu, data_bits: %u, stop_bits: %u, parity: %u\n", data->line_coding.bit_rate, data->line_coding.data_bits,
          data->line_coding.stop_bits, data->line_coding.parity
        );
        break;
      case ARDUINO_USB_CDC_RX_EVENT:
        Serial.printf("CDC RX [%u]:", data->rx.len);
        {
          uint8_t buf[data->rx.len]; //aaaaa
          size_t len = USBSerial.read(buf, data->rx.len);
          Serial.write(buf, len);
          Serial.println();
          recieveCommand(buf, len);
        }
        Serial.println();
        break;
      case ARDUINO_USB_CDC_RX_OVERFLOW_EVENT: Serial.printf("CDC RX Overflow of %d bytes", data->rx_overflow.dropped_bytes); break;

      default: break;
    }
  }
}



void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  USB.VID(0x303A);
  USB.PID(0x8145);
  USB.productName("AudioMixer");
  USB.manufacturerName("TLU Students");

  USB.onEvent(usbEventCallback);
  USBSerial.onEvent(usbEventCallback);

  USBSerial.begin();
  USB.begin();
}

void loop() {
  slider1.tick();
  slider2.tick();
}

enum InCommand {
  REQUEST_INFO = 0x01,
  REQUEST_VOLUME = 0x02,
  SET_VOLUME = 0x03,
};

enum OutCommand {
  SEND_INFO = 0x81,
  SEND_VOLUME = 0x82,
};

static void recieveCommand(uint8_t *buf, size_t len) {
  enum InCommand command = (enum InCommand)buf[0];
  uint8_t channel = buf[1];
  
  uint8_t *value_start = &buf[2];
  size_t value_len = len - 2;

  switch (command) {
    case REQUEST_INFO: {
      for (int i = 0; i < sliderCount; i++) {
        USBSerial.write(SEND_INFO);
        USBSerial.write(i);
        USBSerial.write(slider1.getVolume());
        USBSerial.write(EOF);
      }
      break;
    }
    case REQUEST_VOLUME: {
      uint8_t value = value_start[0];
      Serial.print("Channel: ");
      Serial.print(channel, DEC);
      Serial.print(" Volume: ");
      Serial.println(value, DEC);
      break;
    }
    case SET_VOLUME: {
      uint8_t value = value_start[0];
      Serial.print("Channel: ");
      Serial.print(channel, DEC);
      Serial.print(" Value: ");
      Serial.println(value, DEC);

      if (channel <= sliderCount) {
        sliders[channel - 1].setVolume(value);
      }
      break;
    }
    default:
      Serial.print("Default: ");
      Serial.println(command, HEX);
      break;
  }
}

