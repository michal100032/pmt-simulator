# PMT Simulator Board Verification

Platform: ESP32
Status: Verified
Interface: Python GUI

This repository contains the firmware and control interface for the Photomultiplier Tube (PMT) Simulator Board. This system is a custom electronics module designed to mimic the behavior of a real photomultiplier detector, used for testing, calibration, and FPGA acquisition system development.

## Project Description

The board utilizes a high-resolution Analog Devices AD5361 (14-bit DAC) combined with high-speed transistor stages. The ESP32 acts as the primary controller, managing DAC voltage levels and precise gate switching to generate reproducible PMT-like pulses. This system is essential for validating the electrical behavior of the simulator board before integration with FPGA-based systems.

---

## Building and Running

### 1. Firmware (ESP32)
The project can be built using the Arduino IDE or CLI.

**Compile:**
arduino-cli compile --fqbn esp32:esp32:esp32 .

**Upload:**
arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 .

### 2. Control Interface (Python)
Ensure you have Python 3.9+ installed.

**Install dependencies:**
pip install pyserial customtkinter

**Run the controller:**
python3 ControlPanel.py

---

## Pin Mapping (ESP32)

Based on the latest firmware configuration:

| ESP32 Pin | Function | Description |
| :--- | :--- | :--- |
| 18 | SPI SCK | Serial Clock |
| 23 | SPI MOSI | Master Out Slave In |
| 15 | SPI MISO | Master In Slave Out (Unused) |
| 5 | DAC CS | Chip Select (Active Low) |
| 27 | Gate Trigger 1 | PMT Pulse Output 1 |
| 25 | Gate Trigger 2 | PMT Pulse Output 2 |

---

## Communication Protocol

The PC sends parameters over Serial (115200 Baud) in a comma-separated format.

**Command Format:** V1,V2,DELAY_US,PERIOD_MS\n

* V1, V2: Amplitude voltages (0.0 - 10.0V).
* DELAY_US: Time offset between the two hits (microseconds).
* PERIOD_MS: Cycle repetition time (milliseconds). Set to 0 for single-shot mode.

---

## Pulse Generation Logic

The firmware utilizes a non-blocking state machine (using millis()) to ensure the device remains responsive to new commands while generating high-speed pulses.



1. Command Parsing: ESP32 listens for the \n terminator and decodes the CSV string.
2. DAC Update: Voltage values are converted to 14-bit codes and sent via SPI.
3. Triggering Sequence:
   - Set Trigger 1 (Pin 27) HIGH.
   - Wait for the specified DELAY_US.
   - Set Trigger 2 (Pin 25) HIGH.
   - Maintain HIGH state for PMT_PULSE_WIDTH_US (100us).
   - Reset both triggers to LOW.
4. Repetition: The system waits for the next cycle without using delay(), allowing for real-time parameter updates.

---

## Features

* Real-time Feedback: The GUI displays "ODEBRANO" confirmation from the hardware.
* Dual-Channel Support: Independent amplitude control for two simulated PMT hits.
* Precision Timing: Microsecond-level control over pulse offsets.
* Safety Lock: "Use Generator" interlock to prevent unintended pulsing.

---

## Future Extensions
* External FPGA trigger synchronization.
* Configurable pulse width via GUI.
* Exponential pulse shaping implementation.
