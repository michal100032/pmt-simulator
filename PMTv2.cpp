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


   constexpr float VREF = 10.0f;
   constexpr int DAC_BITS = 14;
   constexpr int DAC_MAX = (1 << DAC_BITS) - 1;
   constexpr int DAC_CHANNEL = 0;


   volatile uint32_t pulseWidth_us  = 100;
   volatile uint32_t pulsePeriod_us = 1000;
   volatile float highVoltage = 8.0f;
   volatile float lowVoltage  = 0.0f;


   SPIClass *spi = nullptr;
   SPIClass spiInstance(HSPI);
   constexpr uint32_t SPI_CLOCK = 2000000;
   SPISettings spiSettings(SPI_CLOCK, MSBFIRST, SPI_MODE1);
}
using namespace CONFIG;


void writeDAC_raw(int channel, uint16_t code) {
   uint32_t mode = 0x0;
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


void setDAC_voltage(int channel, float voltage) {
   if (voltage < 0.0f) voltage = 0.0f;
   if (voltage > VREF) voltage = VREF;
   uint16_t code = round((voltage / VREF) * DAC_MAX);
   writeDAC_raw(channel, code);


   digitalWrite(LDAC_PIN, LOW);
   delayMicroseconds(1);
   digitalWrite(LDAC_PIN, HIGH);
}


void pulseTask(void *pvParameters) {
   for (;;) {
       vTaskDelay(pdMS_TO_TICKS(pulsePeriod_us / 1000));


       setDAC_voltage(DAC_CHANNEL, highVoltage);
       digitalWrite(GATE1_PIN, HIGH);
       digitalWrite(GATE2_PIN, HIGH);


       ets_delay_us(pulseWidth_us);


       digitalWrite(GATE1_PIN, LOW);
       digitalWrite(GATE2_PIN, LOW);
       setDAC_voltage(DAC_CHANNEL, lowVoltage);
   }
}


void setup() {
   Serial.begin(115200);
   delay(500);
   Serial.println("PMT Simulator starting...");




   pinMode(GATE1_PIN, OUTPUT); digitalWrite(GATE1_PIN, LOW);
   pinMode(GATE2_PIN, OUTPUT); digitalWrite(GATE2_PIN, LOW);
   pinMode(LDAC_PIN, OUTPUT);  digitalWrite(LDAC_PIN, HIGH);
   pinMode(RESET_PIN, OUTPUT); digitalWrite(RESET_PIN, HIGH);


   spi = &spiInstance;
   spi->begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, CS_PIN);


   digitalWrite(RESET_PIN, LOW);
   delay(10);
   digitalWrite(RESET_PIN, HIGH);




   setDAC_voltage(DAC_CHANNEL, lowVoltage);


   xTaskCreatePinnedToCore(
       pulseTask,
       "pulseTask",
       4096,
       NULL,
       2,
       NULL,
       1
   );


   Serial.println("Setup done. Pulse task running.");
}




void loop() {
   static uint32_t lastPrint = 0;
   if (millis() - lastPrint > 2000) {
       lastPrint = millis();
       Serial.printf("Pulse width: %u us, period: %u us, highV: %.2f V\n",
                     pulseWidth_us, pulsePeriod_us, highVoltage);
   }
}

