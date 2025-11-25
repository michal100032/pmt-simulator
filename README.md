# Photomultiplier tube simulator board verification

This repository contains firmware for the Photomultiplier Tube (PMT) Simulator Board, a custom electronics module designed to mimic the behavior of a real photomultiplier detector. The board uses a high-resolution Analog Devices AD5361BSTZ DAC together with two high-speed transistor stages that, when properly driven, generate output pulses closely resembling signals from a real PMT.
The ESP32 acts as a controller that sets DAC output voltage levels and precisely times gate transistor switching, allowing reproducible PMT-like pulse generation for testing, calibration, and FPGA firmware development.
This code is primarily intended to validate electrical behavior of the custom PMT simulator board before it is eventually driven by an FPGA system.

## Building and running

The project can be built with Arduino IDE or Arduino CLI. To build with Arduino CLI, run:
```sh
arduino-cli compile --fqbn esp32:esp32:esp32 .
```
And then, to upload:
```sh
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```
## Hardware Overview
- ESP32
- AD5361BSTZ 16-bit DAC, 16-channel,
- Custom PMT - simulator PCB

## Electrical Behavior
The DAC sets an analog voltage representing PMT pulse amplitude. After settings, both transistor gates are activated, generating the final shaped pulse. This output takes the form of a PMT anode current pulse, thereby providing a perfect test case for developing the acquisition system.

## Pulse Generation Sequence
1. Disable both transistor gates 
2. Set DAC channel voltage (highVoltage / lowVoltage)
3. Settle delay on DAC IO
4. Enable transistor gates for pulseWidth_us
5. Return to idle state 

## Pin Mapping

| ESP32 Pin | Function          |
|-----------|-------------------|
| 18        | SPI SCK           |
| 23        | SPI MOSI          |
| 19        | SPI MISO          |
| 27        | Gate Transistor 1 |
| 25        | Gate Transistor 2 |
| 14        | DAC RESET         |
| 26        | DAC LDAC          |
| 5         | DAC CS            |

## Future Extensions
- FPGA trigger synchronization
- Configurable rise/fall shaping
- Multichannel DAC control 
