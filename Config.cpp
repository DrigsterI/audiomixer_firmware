#include "Config.h"
#include <ArduinoJson.h>
#include <string>

void Config::load() {
    preferences.begin("config", true);
    device_name = preferences.getString("device_name", "Default").c_str();
    serial_number = preferences.getString("serial_number", "AM123456").c_str();
    String sliders_json = preferences.getString("sliders", "[]");
    preferences.end();

    StaticJsonDocument<2000> sliders_doc;
    deserializeJson(sliders_doc, sliders_json);

    sliders.clear();
    JsonArray sliders_array = sliders_doc.as<JsonArray>();
    for (JsonObject slider : sliders_array) {
        SliderInfo info;
        info.name = slider["name"] | "Slider";
        info.set_volume_action = slider["set_volume_action"] | "";
        sliders.push_back(info);
    }

    Serial.println("Config loaded");
}

std::string Config::getDeviceName() const {
    return device_name;
}

uint16_t Config::getDeviceVid() const {
    return device_vid;
}

uint16_t Config::getDevicePid() const {
    return device_pid;
}

std::string Config::getDeviceProduct() const {
    return device_product;
}

std::string Config::getDeviceManufacturer() const {
    return device_manufacturer;
}

std::string Config::getSerialNumber() const {
    return serial_number;
}

SliderInfo Config::getSliderInfo(int index) const {
    return sliders[index];
}

const std::vector<SliderInfo>& Config::getSlidersInfo() const {
    return sliders;
}

void Config::setDeviceName(const std::string& name) {
    device_name = name;
    preferences.begin("config", false);
    preferences.putString("device_name", device_name.c_str());
    preferences.end();
}

void Config::setSliderInfo(int index, const SliderInfo& info) {
    while (sliders.size() <= index) {
        sliders.push_back(SliderInfo{"Slider " + std::to_string(sliders.size() + 1), ""});
    }
    sliders[index] = info;
    saveSliders();
}

void Config::saveSliders() {
    StaticJsonDocument<2000> sliders_doc;
    JsonArray sliders_array = sliders_doc.to<JsonArray>();
    for (const auto& slider : sliders) {
        JsonObject obj = sliders_array.createNestedObject();
        obj["name"] = slider.name;
        obj["set_volume_action"] = slider.set_volume_action;
    }
    String sliders_json;
    serializeJson(sliders_doc, sliders_json);
    preferences.begin("config", false);
    preferences.putString("sliders", sliders_json);
    preferences.end();
}
