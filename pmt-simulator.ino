// ============================================================================
// PMT SIMULATOR - Photomultiplier Tube Simulator
// ============================================================================
// This sketch controls a dual-channel voltage output system to simulate
// PMT responses via an AD5361 DAC controller, along with pulse generation.
// ============================================================================

#include <Arduino.h>
#include <SPI.h>

// Pin Configuration
#define PIN_SCK     18    // SPI Serial Clock
#define PIN_MOSI    23    // SPI Master Out Slave In
#define PIN_MISO    15    // SPI Master In Slave Out (unused)
#define PIN_CS      5     // SPI Chip Select
#define PIN_PMT1    27    // First PMT trigger output
#define PIN_PMT2    25    // Second PMT trigger output

// Timing Configuration
#define PMT_PULSE_WIDTH_US  100       // Duration of PMT trigger pulse (microseconds)
#define SPI_CLOCK_HZ        1000000   // SPI clock frequency (1 MHz)

// AD5361 DAC Command Set
#define AD5361_GPIO_FUNC_CODE          ((uint8_t)0b00001101)
#define AD5361_GPIO_LDAC_OUTPUT_HIGH   ((uint8_t)0b11)
#define AD5361_GPIO_LDAC_OUTPUT_LOW    ((uint8_t)0b10)
#define AD5361_MODE_WRITE              ((uint8_t)(0b11 << 6))
#define AD5361_ADDRESS_ALL             ((uint8_t)0)

/**
 * Calculate AD5361 address bits for a specific channel and group
 * 
 * @param channel Channel number (0-7)
 * @param group Group number (0-1)
 * @return Address bits for use in SPI command
 */
#define AD5361_ADDRESS_GET(channel, group) ((uint8_t)((1 << (3 + group)) | ((channel) & 0x0F)))

// DAC Configuration
#define AD5361_OFFSET_CODE  0x2000   // Offset applied to all DAC codes
#define DAC_VREF            5.0f     // Reference voltage (5V)
#define DAC_RESOLUTION      14       // Number of DAC bits
#define DAC_MAX_CODE        ((1 << DAC_RESOLUTION) - 1)  // Maximum DAC output code

// ============================================================================
// UTILITY FUNCTIONS - Voltage Conversion
// ============================================================================

/**
 * Convert a voltage value to AD5361 DAC code
 * 
 * Formula: ((voltage / VREF / 4) * DAC_MAX + OFFSET) << 2
 * 
 * @param voltage Desired output voltage
 * @return 16-bit DAC code for SPI transmission
 */
static inline uint16_t voltage_to_dac_code(float voltage) {
  return ((uint16_t)(voltage / DAC_VREF / 4 * DAC_MAX_CODE) + AD5361_OFFSET_CODE) << 2;
}

// ============================================================================
// SPI COMMUNICATION LAYER
// ============================================================================

/**
 * Send a command to the AD5361 via SPI
 * 
 * @param command_buffer Pointer to 3-byte command buffer
 * @param buffer_size Size of command buffer (typically 3)
 */
static void spi_send_command(const uint8_t* command_buffer, size_t buffer_size) {
  SPI.beginTransaction(SPISettings(
    SPI_CLOCK_HZ,
    MSBFIRST,
    SPI_MODE1
  ));

  // Debug: Log the command being sent
  Serial.printf("SPI: ");
  for (size_t i = 0; i < buffer_size; i++) {
    Serial.printf("0x%02X ", command_buffer[i]);
  }
  Serial.println();

  // Assert CS low, send data, assert CS high
  digitalWrite(PIN_CS, LOW);
  SPI.transfer((uint8_t*)command_buffer, buffer_size);
  digitalWrite(PIN_CS, HIGH);

  SPI.endTransaction();
}

/**
 * Toggle the LDAC (Load DAC) signal to latch output values into the AD5361
 * 
 * LDAC pulse protocol:
 * 1. Assert LDAC low
 * 2. Wait 100us
 * 3. Assert LDAC high
 */
static void trigger_dac_update() {
  // Create LDAC low command
  uint8_t ldac_low_cmd[3] = { 
    AD5361_GPIO_FUNC_CODE, 
    0, 
    AD5361_GPIO_LDAC_OUTPUT_LOW
  };
  spi_send_command(ldac_low_cmd, 3);

  // Hold LDAC low for timing requirement
  delayMicroseconds(100);

  // Create LDAC high command to latch values
  uint8_t ldac_high_cmd[3] = { 
    AD5361_GPIO_FUNC_CODE, 
    0, 
    AD5361_GPIO_LDAC_OUTPUT_HIGH 
  };
  spi_send_command(ldac_high_cmd, 3);
}

// ============================================================================
// HIGH-LEVEL DAC CONTROL
// ============================================================================

/**
 * Set the voltage output on a specific DAC channel
 * 
 * Sends a command to the AD5361 to set the voltage on the specified channel,
 * then immediately triggers an update via LDAC pulse.
 * 
 * @param voltage Desired output voltage
 * @param channel Channel number (0-7)
 * @param group Group number (0-1)
 */
static void set_channel_voltage(float voltage, uint8_t channel, uint8_t group) {
  // Convert voltage to DAC code
  uint16_t dac_code = voltage_to_dac_code(voltage);
  
  // Construct SPI command: [MODE_ADDR, MSB, LSB]
  uint8_t dac_command[3] = { 
    AD5361_MODE_WRITE | AD5361_ADDRESS_GET(channel, group), 
    (dac_code >> 8) & 0xFF,  // Upper 8 bits
    dac_code & 0xFF          // Lower 8 bits
  };
  
  // Send command and update outputs
  spi_send_command(dac_command, 3);
  trigger_dac_update();
}

// ============================================================================
// PMT TRIGGER CONTROL
// ============================================================================

/**
 * Generate a pulse on one or both PMT trigger pins
 * 
 * @param trigger_both If true, trigger both PMT1 and PMT2; if false, only PMT1
 * @param delay_between_pulses_us Delay between PMT1 and PMT2 rising edges (microseconds)
 */
static void trigger_pmt_pulse(bool trigger_both, unsigned int delay_between_pulses_us) {
  // Assert PMT1
  digitalWrite(PIN_PMT1, HIGH);
  
  // If dual-channel pulse, delay before PMT2
  if (trigger_both) {
    delayMicroseconds(delay_between_pulses_us);
    digitalWrite(PIN_PMT2, HIGH);
  }
  
  // Hold pulse for specified width, then release
  delayMicroseconds(PMT_PULSE_WIDTH_US);
  digitalWrite(PIN_PMT1, LOW);
  digitalWrite(PIN_PMT2, LOW);
}

// ============================================================================
// SETUP & INITIALIZATION
// ============================================================================

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  while (!Serial) { }  // Wait for serial port to be ready
  
  // Initialize SPI interface
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  // Configure chip select pin
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);  // CS is active low, so start high

  // Configure PMT trigger pins
  pinMode(PIN_PMT1, OUTPUT);
  pinMode(PIN_PMT2, OUTPUT);
  digitalWrite(PIN_PMT1, LOW);
  digitalWrite(PIN_PMT2, LOW);

  // Initialize LDAC signal to low state
  uint8_t ldac_init_cmd[3] = { 
    AD5361_GPIO_FUNC_CODE, 
    0, 
    AD5361_GPIO_LDAC_OUTPUT_LOW 
  };
  spi_send_command(ldac_init_cmd, 3);
  
  Serial.println("PMT Simulator initialized.");
}

// ============================================================================
// COMMAND PARSING & MAIN LOOP
// ============================================================================

/**
 * Parse a command string from serial input
 * 
 * Command format: "voltage1,voltage2,delay_between_hits_us,delay_between_cycles_ms"
 * Example: "2.5,3.2,500,1000" - Sets CH1 to 2.5V, CH2 to 3.2V, 500us between pulses, 1000ms cycle
 * 
 * @param command_string The string to parse
 * @param pmt1_voltage Output: voltage for PMT1 (channel 0)
 * @param pmt2_voltage Output: voltage for PMT2 (channel 1)
 * @param delay_between_hits_us Output: delay between PMT1 and PMT2 triggers 
 * @param cycle_period_ms Output: total time between successive pulses
 * 
 * @return true if parsing succeeded, false otherwise
 */
static bool parse_command_string(
    const String& command_string,
    float& pmt1_voltage,
    float& pmt2_voltage,
    unsigned int& delay_between_hits_us,
    unsigned int& cycle_period_ms) {
  
  int fields_parsed = sscanf(
    command_string.c_str(), 
    "%f,%f,%u,%u",
    &pmt1_voltage,
    &pmt2_voltage,
    &delay_between_hits_us,
    &cycle_period_ms
  );
  
  return fields_parsed == 4;
}

void loop() {
  static unsigned int pmt_delay_us = 0;        // Delay between PMT pulses
  static unsigned int cycle_period_ms = 0;     // Total cycle period
  static bool is_repeating = false;            // Continue cycles or single shot?
  static bool single_run_complete = true;      // Has single run completed?
  static bool dual_channel = false;            // Use both PMT1 and PMT2?
  static unsigned long last_cycle_time = 0;

  // Wait for next command or cycle timeout
  if (Serial.available() > 0) {
    String received_line = Serial.readStringUntil('\n');
    received_line.trim();
    
    float v1_t, v2_t;
    unsigned int d_t, p_t;
    
    // Attempt to parse command
    if (parse_command_string(received_line, v1_t, v2_t, d_t, p_t)) {
      
      // Update state machine variables
      pmt_delay_us = d_t;
      cycle_period_ms = p_t;
      dual_channel = (pmt_delay_us > 0);
      is_repeating = (cycle_period_ms > 0);
      single_run_complete = false;             // Reset for new cycle
      
      // Apply new voltage settings to both channels
      set_channel_voltage(v1_t, 0, 0);
      set_channel_voltage(v2_t, 1, 0);
      
      // Log parsed values for debugging
      Serial.printf("ODEBRANO: V1=%.2f, V2=%.2f, D=%u, P=%u\n", v1_t, v2_t, pmt_delay_us, cycle_period_ms);
      
      // Exit wait loop to start new cycle
    }
  }

  // Trigger PMT pulse if requested from previous cycle
  if (is_repeating || !single_run_complete) {
    if (cycle_period_ms == 0 || (millis() - last_cycle_time >= cycle_period_ms)) {
      trigger_pmt_pulse(dual_channel, pmt_delay_us);
      last_cycle_time = millis();
      single_run_complete = true;
      if (cycle_period_ms == 0) is_repeating = false;
    }
  }
}
