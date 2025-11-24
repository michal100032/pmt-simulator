#include <Arduino.h>
#include <SPI.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


#define USE_AD536X_LIB
#ifdef USE_AD536X_LIB
#include <AD536x.h>
#endif




namespace CONFIG {
   const int SPI_SCK_PIN  = 18;
   const int SPI_MOSI_PIN = 23;
   const int SPI_MISO_PIN = 19;
   const int CS_PIN       = 5;
   const int LDAC_PIN     = 26;
   const int RESET_PIN    = 14;
   const int GATE1_PIN    = 27;
   const int GATE2_PIN    = 25;


   const float VREF = 10.0f;
   const int DAC_BITS = 14;
   const int DAC_MAX = (1 << DAC_BITS) - 1;


   volatile uint32_t pulseWidth_us  = 100;
   volatile uint32_t pulsePeriod_us = 1000;
   volatile int dacChannel = 0;
   volatile float highVoltage = 8.0f;
   volatile float lowVoltage  = 0.0f;




   SPIClass *spi = NULL;
   SPIClass spiInstance(HSPI);  
   const uint32_t SPI_CLOCK = 2000000;
   SPISettings spiSettings(SPI_CLOCK, MSBFIRST, SPI_MODE1);




}
using namespace CONFIG;




void pulseTask(void * pvParameters);
void writeDAC_raw(int channel, uint16_t code);
void setDAC_voltage(int channel, float voltage);
void ldacPulse();




AD536x dac(CS_PIN, RESET_PIN, LDAC_PIN, RESET_PIN); 




void setup() {
   Serial.begin(115200);
   delay(500);
   Serial.println("Simulator starting.");




   pinMode(CS_PIN, OUTPUT);   digitalWrite(CS_PIN, HIGH);
   pinMode(LDAC_PIN, OUTPUT); digitalWrite(LDAC_PIN, HIGH);
   pinMode(RESET_PIN, OUTPUT); digitalWrite(RESET_PIN, HIGH);




   pinMode(GATE1_PIN, OUTPUT); digitalWrite(GATE1_PIN, LOW);
   pinMode(GATE2_PIN, OUTPUT); digitalWrite(GATE2_PIN, LOW);




   spi = &spiInstance;
   spi->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);


   Serial.println("SPI initialized");




   xTaskCreatePinnedToCore(pulseTask, "pulseTask", 4096, NULL, 2, NULL, 1);


   Serial.println("Setup done.");
}




void pulseTask(void * pvParameters) {
   for (;;) {
 
       vTaskDelay(pdMS_TO_TICKS(pulsePeriod_us / 1000)); 


       setDAC_voltage(dacChannel, highVoltage);
       ldacPulse();


       digitalWrite(GATE1_PIN, HIGH);
       digitalWrite(GATE2_PIN, HIGH);
       ets_delay_us(pulseWidth_us);
       digitalWrite(GATE1_PIN, LOW);
       digitalWrite(GATE2_PIN, LOW);


       setDAC_voltage(dacChannel, lowVoltage);
       ldacPulse();
   }
}




void setDAC_voltage(int channel, float voltage) {
   if (voltage < 0.0f) voltage = 0.0f;
   if (voltage > VREF) voltage = VREF;


   uint16_t code = round((voltage / VREF) * DAC_MAX);
   writeDAC_raw(channel, code);
}




void writeDAC_raw(int channel, uint16_t code) {
   uint32_t mode = 0x0; // tryb write
   uint32_t addr = static_cast<uint32_t>(channel & 0x3F);
   uint32_t data14 = code & 0x3FFF;
   uint32_t word24 = (mode << 22) | (addr << 16) | (data14 << 2);


   uint8_t out[3];
   out[0] = (word24 >> 16) & 0xFF;
   out[1] = (word24 >> 8) & 0xFF;
   out[2] = word24 & 0xFF;


   spi->beginTransaction(spiSettings);
   digitalWrite(CS_PIN, LOW);
   spi->transfer(out[0]);
   spi->transfer(out[1]);
   spi->transfer(out[2]);
   digitalWrite(CS_PIN, HIGH);
   spi->endTransaction();
}




void ldacPulse() {
   digitalWrite(LDAC_PIN, LOW);
   ets_delay_us(1);
   digitalWrite(LDAC_PIN, HIGH);
}




void loop() {
   static uint32_t lastPrint = 0;
   if (millis() - lastPrint > 2000) {
       lastPrint = millis();
       Serial.printf("Pulse width: %u us, period: %u us, highV: %.3f V\n",
           static_cast<unsigned>(pulseWidth_us),
           static_cast<unsigned>(pulsePeriod_us),
           static_cast<double>(highVoltage)
       );
   }
}

