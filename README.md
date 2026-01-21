# Project Documentation
# AudioMixer - Motorized Slider Volume Controller
Team members: Gabriel Jõe, Mihhail Zolotarjov, Maria Rjabtseva
 
# 1. Project goal and short device description
## What is this device and what does it do?
The goal of this project is to create a motorized audio mixer that allows controlling PC application volume using physical motorized sliders.
The device works in two directions:
Moving a physical slider changes the volume on the PC.
Changing the volume on the PC automatically moves the corresponding physical slider.
This solves the problem of desynchronization between software volume levels and physical controllers, which is common in traditional MIDI or analog mixers.
Main components:
ESP32-S3 microcontroller
Motorized sliders (DC motor + potentiometer)
Motor drivers
USB-C connection to PC

# 2. Inputs
What inputs does the system read and how are they measured?
Slider position → Measured using potentiometers connected to ESP32 analog inputs (ADC, 0-4095)
USB serial commands from PC → Received via USB CDC interface → Commands include volume updates and device information requests

# 3. Outputs
What does the system control or change?
Slider movement → DC motors controlled through motor driver (PWM + direction pins)
Volume data sent to PC → USB CDC serial communication

# 4. Device behavior requirements
```
System logic described as rules (“If X, then Y”)
If the user moves a physical slider:
  The potentiometer value changes
  The firmware detects the change
  Slider position is converted to volume (0-100)
  SEND_VOLUME command is sent to the PC
If the PC sends a SET_VOLUME command:
  The target position for the slider is calculated
  The motor moves the slider to the target using PID control
If the slider reaches the target position:
  The motor stops
  PID integral error is reset
If USB is not connected:
  No commands are sent
  Slider control continues locally
```

# 5. List of physical system components (Hardware)

| Component         | Quantity        | Pins            | Description                              |
|-------------------|-----------------|-----------------|------------------------------------------|
| ESP32-S3          | 1               | –               | Main microcontroller with USB CDC        |
| DC motors         | 5 (Integrated)  | 12, 10, 14, 3…  | Slider control                           |
| Potentiometers    | 5 (Integrated)  | 1-5           | Slider position sensing                  |
| Motor driver      | 2               | IN1 / IN2 / EN  | H-bridge motor control                  |
| USB-C cable       | 1               | USB CDC         | Power and communication                 |
| 3D-printed case   | 1               | –               | PLA / ABS material                      |


# 6. Wiring diagram
Textual description of connections:
Example for one slider:
<img width="1509" height="1124" alt="scheme" src="https://github.com/user-attachments/assets/1bcb9155-cc37-4333-9481-5e458b512ef4" />

# 7. Software (firmware logic)
Firmware structure and logic overview
Main firmware file audiomixer_firmware.ino:
Main loop logic:
```
void loop() {
  for (Slider &slider : sliders) {
    slider.tick();
  }
  commandTick();
}
```
Software modules
Slider.cpp / Slider.h
Handles motor control, position reading, and PID regulation
Command.cpp / Command.h
Implements a non-blocking USB serial protocol with command queueing
Config.cpp / Config.h
Manages persistent configuration using ESP32 NVS and JSON
USBCallback.ino
Handles USB CDC events and incoming data
Communication protocol
```
PC → Device
REQUEST_INFO = 0x01
SET_VOLUME   = 0x02

Device → PC
SEND_INFO   = 0x81
SEND_VOLUME = 0x82
```

Data is sent in the format:
```
[COMMAND][CHANNEL][DATA][EOF]
```


# Media
Working prototype

![20260117_013753-ezgif com-video-to-gif-converter](https://github.com/user-attachments/assets/4fba063c-898e-4007-9bbc-87f5980f0b12)



# Links
Figma: https://www.figma.com/design/y4uMa08nGjPaki2fUYyuy0/Audiomixer_app

Repository: https://github.com/DrigsterI/audiomixer_firmware/tree/master

Arduino documentation: https://www.arduino.cc

ESP32 documentation: https://docs.espressif.com

Slider documentation: https://tech.alpsalpine.com/e/products/detail/RSA0N11M9A0K/
