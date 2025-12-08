// For I2C communication (Mandatory Challenge: Embedded Communications)
#include <Wire.h>

// For Power Optimisation (Mandatory Challenge: Power Optimisation)
#include <avr/sleep.h>
#define AIR_QUALITY_PIN A0 // Analog pin for MQ-135 or similar
#define LIGHT_SENSOR_PIN A1 // Analog pin for LDR
#define DHT_PIN 2          // Digital pin for DHT sensor (Temp/Humid)

// Define I2C address for a display (e.g., SSD1306 OLED)
#define OLED_I2C_ADDRESS 0x3C
// Structure to hold all sensor data (improves code organization)
struct SensorData {
  float airQuality_PPM;
  float light_Percent;
  float temperature_C;
  float humidity_Percent;
  int rawAirQuality; // Raw ADC value (0-1023)
  int rawLightIntensity; // Raw ADC value (0-1023)
};

float readAirQuality() {
  // Simulate an analog read from the sensor
  int rawValue = analogRead(AIR_QUALITY_PIN);
  currentData.rawAirQuality = rawValue; // Store raw value
  
  // This simulates the signal conditioning and calibration process.
  float mappedValue = map(rawValue, 0, 1023, 100, 620) / 10.0;
  
  // Ensure the value is strictly within the specified range
  return constrain(mappedValue, 10.0, 62.0);
}
float readLightIntensity() {
  int rawValue = analogRead(LIGHT_SENSOR_PIN);
  currentData.rawLightIntensity = rawValue; // Store raw value
  
  // Map the raw value to a range slightly above 99.5 to allow for the max value
  float mappedValue = map(rawValue, 0, 1023, 0, 1000) / 10.0;
  
  // Ensure the value is strictly within the specified range
  return constrain(mappedValue, 0.0, 99.5);
}
void readTempAndHumidity(SensorData* data) {

  static float simTemp = 20.0;
  static float simHumid = 50.0;

  // Apply a small random change
  simTemp += random(-5, 6) / 10.0; // Change by -0.5 to +0.5
  simHumid += random(-10, 11) / 10.0; // Change by -1.0 to +1.0
  
  // Constrain the simulated values to the required ranges
  data->temperature_C = constrain(simTemp, 0.0, 35.0);
  data->humidity_Percent = constrain(simHumid, 25.0, 90.0);
  
  // Update the static variables for the next iteration
  simTemp = data->temperature_C;
  simHumid = data->humidity_Percent;
}

void processMoodAndOutput(const SensorData& data) {
  // Define the mood, LED color, and emoji strings
  String mood;
  String ledColor;
  String emoji;
  
  // 1. IF AQ < 300 (Raw ADC value)
  if (data.rawAirQuality < 300) {
    mood = "poor_air";
    ledColor = "blue";
    emoji = "😷";
  } 
  // 2. ELSE IF LIGHT < 800 (Scaled to raw ADC value: 800/3000 * 1023 ~= 273)
  else if (data.rawLightIntensity < 273) {
    mood = "dark";
    ledColor = "warm_white";
    emoji = "🌙";
  } 
  // 3. ELSE IF TEMP > 28 (°C)
  else if (data.temperature_C > 28.0) {
    mood = "hot";
    ledColor = "cool_blue";
    emoji = "🥵";
  } 
  // 4. ELSE (Excellent conditions)
  else {
    mood = "excellent";
    ledColor = "green";
    emoji = "😄";
  }

  // Output the result to the Serial Monitor
  Serial.println(F("--- Downstream Mood Analysis ---"));
  Serial.print(F("Mood: "));
  Serial.print(mood);
  Serial.print(F(" ("));
  Serial.print(emoji);
  Serial.println(F(")"));
  Serial.print(F("Suggested LED Color: "));
  Serial.println(ledColor);
  Serial.println(F("--------------------------------"));
}
void outputToSerial(const SensorData& data) {
  Serial.println(F("-----------------------------------"));
  Serial.println(F("Air Quality Monitoring Report"));
  Serial.print(F("Air Quality (PPM): "));
  Serial.println(data.airQuality_PPM, 1); // 1 decimal place
  Serial.print(F("Light Intensity (%): "));
  Serial.println(data.light_Percent, 1);
  Serial.print(F("Temperature (°C): "));
  Serial.println(data.temperature_C, 1);
  Serial.print(F("Humidity (%): "));
  Serial.println(data.humidity_Percent, 1);
  Serial.println(F("-----------------------------------"));
}

void outputToI2CDisplay(const SensorData& data) {
  // Start I2C transmission to the display address
  Wire.beginTransmission(OLED_I2C_ADDRESS);
  
  Wire.write(0x01); // Status: Data Sent
  
  // End I2C transmission
  Wire.endTransmission();
  
  // For debugging the I2C communication logic
  Serial.println(F("[I2C] Data transmission simulated to OLED at 0x3C."));
}

void enterSleepMode() {
  Serial.println(F("[Power] Entering low-power sleep mode..."));
  Serial.flush(); // Wait for all serial data to be sent

  // 1. Set the sleep mode (e.g., SLEEP_MODE_PWR_DOWN is the deepest sleep)
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  
  // 2. Enable sleep mode
  sleep_enable();
  
  // 3. Disable ADC to save power
  // ADCSRA &= ~_BV(ADEN); 
  
  // 4. Enter sleep
  sleep_cpu();
  
  // --- Execution resumes here after wake-up ---
  
  // 5. Disable sleep mode
  sleep_disable();
  
  // 6. Re-enable ADC if it was disabled
  // ADCSRA |= _BV(ADEN); 
  
  Serial.println(F("[Power] Waking up from sleep mode."));
}

// =============================================================================
// 6. ARDUINO SETUP AND MAIN LOOP
// =============================================================================

void setup() {
  // Initialize Serial Communication (Human-readable interface)
  Serial.begin(9600);
  while (!Serial) { ; } // Wait for serial port to connect (needed for Leonardo/Micro)
  Serial.println(F("--- Embedded Systems CW2: Air Quality Monitor Initialized ---"));
  
  // Initialize I2C Communication (Embedded Communications)
  Wire.begin();
  Serial.println(F("[I2C] Wire library initialized."));
  
  // Initialize sensor pins (set as inputs)
  pinMode(AIR_QUALITY_PIN, INPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  pinMode(DHT_PIN, INPUT); // DHT pin is usually an input/output for the 1-wire protocol
  
  // Initialize random seed for simulation
  randomSeed(analogRead(0));
}

void loop() {
  
  currentData.airQuality_PPM = readAirQuality();
  currentData.light_Percent = readLightIntensity();
  readTempAndHumidity(&currentData); // Updates Temp and Humidity in the struct
  
  outputToSerial(currentData);
  outputToI2CDisplay(curentData);
  
  processMoodAndOutput(currentData);
  
  if (currentData.airQuality_PPM > 50.0) {
    Serial.println(F("[Automation] Air Quality is poor (>50 PPM). Activating ventilation system."));
    // In a real system, a relay pin would be set HIGH here.
  }
  
  // Run the main loop for 5 seconds, then enter sleep mode for a period
  delay(5000); // Wait 5 seconds before next reading
  
  // To demonstrate power optimization, we enter sleep mode every 5 cycles
  static int cycleCount = 0;
  cycleCount++;
  
  if (cycleCount >= 5) {
    enterSleepMode();
    // The delay below simulates the time spent in sleep mode.
    delay(10000); // Simulate 10 seconds of sleep time
    cycleCount = 0;
  }
}
