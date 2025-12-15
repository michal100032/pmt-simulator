#include <Arduino.h>
#include <SPI.h>

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
  constexpr uint16_t DAC_MAX = (1 << DAC_BITS) - 1;

  volatile uint32_t pulseWidth_us  = 100;
  volatile uint32_t pulsePeriod_us = 1000;
  volatile float highVoltage = 8.0f;
  volatile float lowVoltage  = 0.0f;
}

using namespace CONFIG;

SPIClass spi(HSPI);

static inline void csLow()  { GPIO.out_w1tc = (1UL << CS_PIN); }
static inline void csHigh() { GPIO.out_w1ts = (1UL << CS_PIN); }

static inline void ldacLow()  { GPIO.out_w1tc = (1UL << LDAC_PIN); }
static inline void ldacHigh() { GPIO.out_w1ts = (1UL << LDAC_PIN); }

static inline void gatesOff() {
  GPIO.out_w1tc = (1UL << GATE1_PIN) | (1UL << GATE2_PIN);
}

static inline void gate1On() {
  GPIO.out_w1ts = (1UL << GATE1_PIN);
}

static inline void gate2On() {
  GPIO.out_w1ts = (1UL << GATE2_PIN);
}

static inline uint16_t voltageToCode(float v) {
  if (v <= 0.0f) return 0;
  if (v >= VREF) return DAC_MAX;
  return (uint16_t)((v / VREF) * DAC_MAX + 0.5f);
}

static inline void spiWrite24(uint32_t word) {
  csLow();
  spi.transfer((word >> 16) & 0xFF);
  spi.transfer((word >> 8)  & 0xFF);
  spi.transfer(word & 0xFF);
  csHigh();
}

static inline void dacIOUpdate() {
  ldacLow();
  delayMicroseconds(1);
  ldacHigh();
}

static inline void dacReset() {
  GPIO.out_w1tc = (1UL << RESET_PIN);
  delayMicroseconds(10);
  GPIO.out_w1ts = (1UL << RESET_PIN);
  delayMicroseconds(10);
}

static inline void dacWriteChannel0(uint16_t code) {
  uint32_t frame =
      (0b0001 << 20) |   // Write to DAC register
      (0b0000 << 16) |   // Channel 0
      (code & 0x3FFF);

  spiWrite24(frame);
  dacIOUpdate();
}

static inline void generatePulse(float voltage, uint32_t width_us) {
  gatesOff();
  dacWriteChannel0(voltageToCode(voltage));
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

  csHigh();
  ldacHigh();
  gatesOff();

  spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);
  spi.beginTransaction(SPISettings(20000000, MSBFIRST, SPI_MODE1));

  dacReset();
  dacWriteChannel0(voltageToCode(lowVoltage));

  Serial.println("PMT Simulator – bare metal DAC ready");
}

void loop() {
  uint32_t t0 = micros();
  generatePulse(highVoltage, pulseWidth_us);
  while ((micros() - t0) < pulsePeriod_us) {}
}

