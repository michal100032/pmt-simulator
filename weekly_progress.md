# Weekly_progress

## Week 1:
## Objective:
Project initiation and hardware familiarization
## Tasks:
- Defined project scope: simulate PMT signals using ESP32 and AD5361 DAC
- Verified hardware components: ESP32, DAC module, transistor gates, and connecting circuitry
- Set up development environment: Arduino IDE and required toolchains
- Initial testing of GPIO pins and SPI connectivity

## Week 2:
## Objective:
Establish reliable communication between ESP32 and AD5361 DAC
## Tasks:
Installed and configured AD536x Arduino library
Verified ESP32-to-DAC SPI pin mapping and control pins
- Conducted initial SPI communication tests to ensure DAC responds correctly
- Implemented basic DAC initialization sequence (dac.begin(), dac.softReset())

## Week 3:
## Objective:
Generate stable, controlled analog output voltage from DAC
## Tasks:
- Tested DAC voltage output using setVoltageHold() function
- Verified voltage accuracy at multiple reference levels (0–10 V)
- Calibrated highVoltage and lowVoltage parameters for pulse simulation
- Documented response linearity and voltage stability over time

## Week 4:
## Objective:
Implement precise and cyclic switching of transistor gates for pulse generation
## Tasks:
- Developed hardware timer routines using hw_timer_t and onTimer() callbacks
- Tested pulseTask synchronization using semaphores and ensured consistent wake-ups at 1 ms intervals
- Measured pulse jitter and delay using ets_delay_us() and optimized timing accuracy
- Verified gate activation sequence to ensure proper PMT-like pulse shaping

## Week 5:
## Objective:
Achieve well-shaped pulses with controlled amplitude, width, and clock synchronization
## Tasks:
- Finalized pulseTask implementation for real-time pulse generation
- Integrated dynamic parameter adjustment: highVoltage and pulseWidth_us modifiable in loop()
- Verified pulse consistency and reproducibility across multiple test cycles
- Conducted debugging and fine-tuning of pulse edges to reduce overshoot and timing errors


