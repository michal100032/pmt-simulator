#include <Arduino.h>
#include <SPI.h>
#include "AD536x.h"
#include "soc/gpio_struct.h"
#include "driver/gpio.h"

namespace CONFIG {
  constexpr int SPI_SCK_PIN  = 18;
  constexpr int SPI_MOSI_PIN = 23;
  constexpr int SPI_MISO_PIN = 19;

  constexpr int CS_PIN       = 5;
  constexpr int LDAC_PIN     = 26;
  constexpr int RESET_PIN    = 14;

  constexpr int GATE1_PIN    = 27;
  constexpr int GATE2_PIN    = 25;

  constexpr float VREF = 10.0f;

  volatile uint32_t pulseWidth_us  = 100;
  volatile uint32_t pulsePeriod_us = 1000;

  volatile float highVoltage = 8.0f;
  volatile float lowVoltage  = 0.0f;
}

using namespace CONFIG;

AD536x dac(CS_PIN, RESET_PIN, LDAC_PIN, RESET_PIN);

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

  dac.setVoltageHold(BANK0, CH0, voltage);
  dac.IOUpdate();

  delayMicroseconds(2);

  gate1On();
  delayMicroseconds(width_us);
  gate2On();
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  pinMode(CS_PIN, OUTPUT);
  pinMode(LDAC_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  pinMode(GATE1_PIN, OUTPUT);
  pinMode(GATE2_PIN, OUTPUT);

  digitalWrite(CS_PIN, HIGH);
  digitalWrite(LDAC_PIN, HIGH);
  digitalWrite(RESET_PIN, HIGH);

  gatesOff();

  SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);
  SPI.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE1));

  dac.setGlobalVref(BANKALL, VREF);
  dac.setVoltageHold(BANK0, CH0, lowVoltage);
  dac.IOUpdate();

  Serial.println("PMT Simulator ready – ESP32 + AD5361");
}

void loop() {
  uint32_t t0 = micros();
  generatePulse(highVoltage, pulseWidth_us);
  while ((micros() - t0) < pulsePeriod_us) {}
}


