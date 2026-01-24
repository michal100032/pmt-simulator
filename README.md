# PMT Simulator Board Verification

Platform: ESP32    
Interface: Python GUI  

This repository contains the firmware and control interface for the Photomultiplier Tube (PMT) Simulator Board. This system is a custom electronics module designed to mimic the behavior of a real photomultiplier detector, used for testing, calibration, and FPGA acquisition system development.

## Project Description

The board utilizes a high-resolution Analog Devices AD5361 (14-bit DAC) combined with high-speed transistor stages. The ESP32 acts as the primary controller, managing DAC voltage levels and precise gate switching to generate reproducible PMT-like pulses. This system is essential for validating the electrical behavior of the simulator board before integration with FPGA-based systems.

---

## Building and Running

### 1. Firmware (ESP32)

**Compile:**  
```bash
arduino-cli compile --fqbn esp32:esp32:esp32 .
```

**Upload:** 
```bash 
arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32 .
```

---

### 2. Control Interface (Python)

Ensure you have Python 3.9+ installed.

**Install dependencies:** 
```bash 
pip install pyserial customtkinter
```

**Run the controller:**  
```bash
python3 ControlPanel.py
```

---

## Pin Mapping (ESP32)

| ESP32 Pin | Function | Description|
|----------|----------|------------------------|
| 18 | SPI SCK  | Serial Clock                 |
| 23 | SPI MOSI | Master Out Slave In          |
| 15 | SPI MISO | Master In Slave Out (Unused) |
| 5  | DAC CS   | Chip Select (Active Low)     |
| 27 | Gate Trigger 1 | PMT Pulse Output 1     |
| 25 | Gate Trigger 2 | PMT Pulse Output 2     |

---

## Communication Protocol

The PC sends parameters over Serial (115200 Baud) in a comma-separated format.

**Command format:**  
`V1,V2,DELAY_US,PERIOD_MS\n`

- **V1, V2** – Amplitude voltages (0.0–10.0 V)  
- **DELAY_US** – Time offset between the two hits (microseconds)  
- **PERIOD_MS** – Cycle repetition time (milliseconds, 0 = single-shot)

---

## Pulse Generation Logic

The firmware uses a non-blocking state machine (`millis()`) to remain responsive during high-speed pulse generation.

1. Command parsing via `\n` terminator
2. DAC update via SPI
3. Triggering sequence:
   - Trigger 1 (Pin 27) HIGH
   - Wait `DELAY_US`
   - Trigger 2 (Pin 25) HIGH
   - Hold for `PMT_PULSE_WIDTH_US` (100 µs)
   - Reset both triggers LOW
4. Repetition without `delay()`

---

## Features

- Real-time GUI feedback ("ODEBRANO")
- Dual-channel amplitude control
- Microsecond-level timing precision
- Safety interlock ("Use Generator")

---

## Future Extensions

- External FPGA trigger synchronization
- GUI-configurable pulse width
- Exponential pulse shaping

