#ifndef CONFIG_H
#define CONFIG_H

#include <Preferences.h>
#include <ArduinoJson.h>
#include <string>

struct SliderInfo {
  std::string name;
  std::string set_volume_action;
};

class Config {
private:
  Preferences preferences;
  std::string device_name;
  const uint16_t device_vid = 0x303A;
  const uint16_t device_pid = 0x8145;
  const std::string device_product = "AudioMixer";
  const std::string device_manufacturer = "TLU Students";
  std::string serial_number;
  std::vector<SliderInfo> sliders;

public:
  void load();
  std::string getDeviceName() const;
  uint16_t getDeviceVid() const;
  uint16_t getDevicePid() const;
  std::string getDeviceProduct() const;
  std::string getDeviceManufacturer() const;
  std::string getSerialNumber() const;
  SliderInfo getSliderInfo(int index) const;
  const std::vector<SliderInfo>& getSlidersInfo() const;
  void setDeviceName(const std::string& name);
  void setSliderInfo(int index, const SliderInfo& info);

private:
  void saveSliders();
};

#endif