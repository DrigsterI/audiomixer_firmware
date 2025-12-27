#include "USB.h"
#include "Preferences.h"
#include "ArduinoJson.h"

#include "Slider.h"
#include "Config.h"
#include "Command.h"

Preferences preferences;
USBCDC USBSerial;
Config config;

Slider slider1 = Slider(12, 11, 14, 3, [](int value) {
  queueCommand(SEND_VOLUME, 1, (uint8_t)value);
});
Slider slider2 = Slider(10, 9, 13, 8, [](int value) {
  queueCommand(SEND_VOLUME, 2, (uint8_t)value);
});
std::vector<Slider> sliders = {slider1, slider2};

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  delay(1000);
  Serial.println();

  config.load();
  
  const std::vector<SliderInfo> config_sliders = config.getSlidersInfo();

  Serial.print("Config sliders: ");
  Serial.println(config_sliders.size());
  Serial.print("Sliders: ");
  Serial.println(sliders.size());

  if(config_sliders.size() < sliders.size()){
    Serial.println("Resizing sliders array");
    for (int i = 0; i < sliders.size(); i++) {
      SliderInfo slider = {
        .name = "Slider " + std::to_string(i + 1),
        .set_volume_action = ""
      };
      config.setSliderInfo(i, slider);
    }
  }

  Serial.print("Device vid: ");
  Serial.println(config.getDeviceVid(), HEX);
  Serial.print("Device pid: ");
  Serial.println(config.getDevicePid(), HEX);

  USB.VID(config.getDeviceVid());
  USB.PID(config.getDevicePid());
  USB.productName(config.getDeviceProduct().c_str());
  USB.manufacturerName(config.getDeviceManufacturer().c_str());
  USB.serialNumber(config.getSerialNumber().c_str());
  USB.firmwareVersion(1);

  USB.onEvent(usbEventCallback);
  USBSerial.onEvent(usbEventCallback);

  USB.begin();
  USBSerial.begin();
}

void loop() {
  for (Slider &slider : sliders) {
    slider.tick();
  }
  
  commandTick();
}