# Photomultiplier tube simulator board verification

The aim of this project is to test a custom photomultiplier simulator board which eventually is expected to be controlled by an FPGA. The repo contains code for ESP32 which controls a DAC and two transistors present on the board in a way that results in a photomultiplier-like signal at the output. To control the DAC [AD536x-arduino library](https://github.com/JQIamo/AD536x-arduino) is used.

## Building and running

The project can be built with Arduino IDE or Arduino CLI. To build with Arduino CLI, run:
```sh
arduino-cli compile --fqbn esp32:esp32:esp32 .
```
And then, to upload:
```sh
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```