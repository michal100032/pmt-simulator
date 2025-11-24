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


   constexpr float VREF = 10.0f;
   constexpr int DAC_BITS = 14;
   constexpr int DAC_MAX = (1 << DAC_BITS) - 1;
   constexpr AD536x_ch_t DAC_CHANNEL = CH0;


   volatile uint32_t pulseWidth_us  = 100;
   volatile uint32_t pulsePeriod_us = 1000;
   volatile float highVoltage = 8.0f;
   volatile float lowVoltage  = 0.0f;

}
using namespace CONFIG;

SPIClass spi(HSPI);
AD536x dac(spi, RESET_PIN, LDAC_PIN, RESET_PIN); 


void setup() {
   Serial.begin(115200);

   // Wait for Serial to be ready
   while(!Serial) { ; }
   Serial.println("PMT Simulator starting...");

   // Initialize gate control pins
   pinMode(GATE1_PIN, OUTPUT); digitalWrite(GATE1_PIN, LOW);
   pinMode(GATE2_PIN, OUTPUT); digitalWrite(GATE2_PIN, LOW);

   // set reference voltage for DAC
   dac.setGlobalVref(BANKALL, VREF);

   // Initialize SPI
   spi.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);
}

static void generatePulse(double voltage, uint32_t width_us) {
   
   // shut off both transistors first
   digitalWrite(GATE1_PIN, LOW);
   digitalWrite(GATE2_PIN, LOW);
   delayMicroseconds(500);
   
   // set DAC to desired voltage 
   dac.setVoltageHold(BANK0, DAC_CHANNEL, voltage);
   dac.IOUpdate();

   delayMicroseconds(500);

   // turn on both transistors for desired pulse width
   digitalWrite(GATE1_PIN, HIGH);
   delayMicroseconds(width_us);
   digitalWrite(GATE2_PIN, HIGH);
}

void loop() {
   Serial.println("Generating pulse...");
   generatePulse(highVoltage, pulseWidth_us);
   Serial.println("Pulse done.");
   delay(1000);
}

