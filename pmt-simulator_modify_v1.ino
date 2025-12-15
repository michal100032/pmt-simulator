#include <Arduino.h>
#include <SPI.h>
#include "AD536x.h"

namespace CONFIG {
  constexpr int SPI_SCK_PIN  = 18;
  constexpr int SPI_MOSI_PIN = 23;
  constexpr int SPI_MISO_PIN = 19;
  constexpr int CS_PIN       = 5;
  constexpr int LDAC_PIN     = 26;
  constexpr int RESET_PIN    = 14;
  constexpr int GATE1_PIN    = 27;
  constexpr int GATE2_PIN    = 25;

  constexpr float VREF       = 10.0f;
  constexpr int DAC_BITS     = 14;
  constexpr int DAC_MAX      = (1 << DAC_BITS) - 1;

  constexpr AD536x_bank_t BANK = BANK0;
  constexpr AD536x_ch_t   CH   = CH0;

  volatile uint32_t pulseWidth_us  = 100;
  volatile uint32_t pulsePeriod_us = 1000;
  volatile float highVoltage = 8.0f;
  volatile float lowVoltage  = 0.0f;
}

using namespace CONFIG;

SPIClass spi(HSPI);
AD536x dac(spi, CS_PIN, LDAC_PIN, RESET_PIN);

static inline uint16_t voltageToCode(float v) {
  if (v <= 0.0f) return 0;
  if (v >= VREF) return DAC_MAX;
  return static_cast<uint16_t>((v / VREF) * DAC_MAX + 0.5f);
}

static inline void dacWrite(float voltage) {
  dac.setValue(BANK, CH, voltageToCode(voltage));
  dac.IOUpdate();
}

static inline void gatesOff() {
  GPIO.out_w1tc = (1UL << GATE1_PIN) | (1UL << GATE2_PIN);
}

static inline void gate1On() {
  GPIO.out_w1ts = (1UL << GATE1_PIN);
}

static inline void gate2On() {
  GPIO.out_w1ts = (1UL << GATE2_PIN);
}

static void generatePulse(float voltage, uint32_t width_us) {
  gatesOff();
  dacWrite(voltage);
  delayMicroseconds(2);
  gate1On();
  delayMicroseconds(width_us);
  gate2On();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(GATE1_PIN, OUTPUT);
  pinMode(GATE2_PIN, OUTPUT);
  gatesOff();

  spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);

  dac.reset();
  dac.setGlobalVref(BANKALL, VREF);
  dacWrite(lowVoltage);

  Serial.println("PMT Simulator ready");
}

void loop() {
  uint32_t t0 = micros();
  generatePulse(highVoltage, pulseWidth_us);
  while ((micros() - t0) < pulsePeriod_us) {}
}

