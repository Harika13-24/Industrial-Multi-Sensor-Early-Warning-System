/**
 * ==================================================================================
 * PROJECT: Industrial Multi-Sensor Early Warning & Protection System
 * ARCHITECTURE: State-Driven Embedded Firmware with Threshold Telemetry
 * TARGET HARDWARE: Espressif ESP32 Microcontroller
 * 
 * DESCRIPTION:
 * This production-grade firmware implements a real-time predictive safety system
 * for industrial machinery (e.g., turbines, boilers). It monitors physical anomalies
 * using a multi-sensor bus array and triggers immediate physical isolation if safe 
 * operational limits are exceeded.
 * ==================================================================================
 */

#include <Wire.h>                 // Built-in library handling I2C communication protocol
#include <Adafruit_MPU6050.h>     // Drivers for handling the 3-axis accelerometer sensor
#include <Adafruit_Sensor.h>     // Unified master sensor library by Adafruit
#include <DHT.h>                  // Driver for handling the digital temperature sensor
#include <LiquidCrystal_I2C.h>    // Controller library for the I2C alphanumeric display

/* ==========================================
 * HARDWARE CONFIGURATION & PIN MAPPING
 * ========================================== */
#define DHTPIN 4                  // Digital GPIO pin reading temperature data pulses
#define DHTTYPE DHT22             // Specifying the precise sensor module version (AM2302)
#define RELAY_TRIP_PIN 12         // GPIO output activating the industrial isolation relay
#define BUZZER_PIN 13             // GPIO output driving the local audial warning siren
#define SYSTEM_LED 14             // GPIO output for the visual "System Healthy" heart-beat LED

/* ==========================================
 * INDUSTRY DESIGN CRITERIA & CONSTANTS
 * ========================================== */
// Threshold parameters calculated using standard machinery tolerance specifications
const float VIBRATION_THRESHOLD = 2.50; // Max allowed vibration acceleration change in m/s^2
const float TEMP_MAX_THRESHOLD = 60.0;  // Critical operating temperature ceiling in Celsius

/* ==========================================
 * DRIVER & INSTANCE INITIALIZATIONS
 * ========================================== */
Adafruit_MPU6050 mpu;                   // Creates an object instance for the Accelerometer
DHT dht(DHTPIN, DHTTYPE);               // Initializes the Temperature sensor with pin mappings
LiquidCrystal_I2C lcd(0x27, 16, 2);     // Sets up the LCD using its hexadecimal I2C address

/* ==========================================
 * GLOBAL SYSTEM VARIABLES (STATE REGISTERS)
 * ========================================== */
bool systemFault = false;               // Master flag storing current system health state
float baselineX, baselineY, baselineZ;  // Variables storing structural rest values during startup

/**
 * setup() runs exactly once when the microcontroller boots up.
 * Used for peripheral initialization, calibration, and safety validation.
 */
void setup() {
  // Open serial hardware communication channel for active telemetry output
  Serial.begin(115200);
  
  // Explicitly configure digital hardware pins as input or output states
  pinMode(RELAY_TRIP_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SYSTEM_LED, OUTPUT);
  
  // INITIAL SAFETY STATE: Machinery trip relay OFF, System Status LED ON
  digitalWrite(RELAY_TRIP_PIN, LOW); 
  digitalWrite(SYSTEM_LED, HIGH);

  // Bring up the Liquid Crystal Display module
  lcd.init();
  lcd.backlight();
  lcd.print("System Loading...");

  // Validate presence of physical hardware over the I2C interface bus
  if (!mpu.begin()) {
    Serial.println("CRITICAL ERROR: Failed to detect MPU6050 Accelerometer!");
    lcd.setCursor(0,1);
    lcd.print("MPU6050 Error");
    while (1) { delay(10); } // Hard halt system if a key safety sensor is missing
  }
  
  dht.begin(); // Initialize communication with the DHT sensor
  
  // DEVICE CALIBRATION: Fetch structural resting forces to filter out ambient room vibrations
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  baselineX = a.acceleration.x;
  baselineY = a.acceleration.y;
  baselineZ = a.acceleration.z;

  lcd.clear();
  lcd.print("System Nominal");
  delay(1500); // Allow hardware states to fully stabilize
}

/**
 * loop() executes continuously in an infinite cycle.
 * Contains the main logic loop: Read Sensors -> Process Data -> Evaluate Safety State -> Respond.
 */
void loop() {
  // Create structures to store temporary incoming raw register values
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp); // Read current acceleration values from the sensor chip

  float currentTemperature = dht.readTemperature(); // Capture temperature conversion value
  
  // ALGORITHM: Calculate 3D Vector Delta magnitude of current vibration deviation
  float diffX = abs(a.acceleration.x - baselineX);
  float diffY = abs(a.acceleration.y - baselineY);
  float diffZ = abs(a.acceleration.z - baselineZ);
  // Three-dimensional Euclidean distance equation: d = sqrt(x^2 + y^2 + z^2)
  float vibrationMagnitude = sqrt(diffX*diffX + diffY*diffY + diffZ*diffZ);

  // FAILSAFE CHECK: If a wire loose breaks a sensor link, catch the error instead of reading trash data
  if (isnan(currentTemperature)) {
    Serial.println("Warning: Failed to read data from DHT sensor array!");
    return; // Exit current cycle prematurely to avoid evaluating wrong data
  }

  /* ==========================================
   * CONTROL LOGIC & DECISION ENGINE
   * ========================================== */
  // Evaluate if any sensor reading violates safe industrial boundaries
  if (vibrationMagnitude > VIBRATION_THRESHOLD || currentTemperature > TEMP_MAX_THRESHOLD) {
    // Structural boundary violated! If system hasn't tripped yet, execute trip routine
    if (!systemFault) {
      triggerEmergencyTrip(vibrationMagnitude, currentTemperature);
    }
  } else {
    // Operational variables within safe specifications. Reset system if recovering from fault.
    if (systemFault) {
      clearEmergencyState();
    }
    // Stream healthy real-time machine telemetry over Serial console and LCD
    updateNormalTelemetry(vibrationMagnitude, currentTemperature);
  }

  delay(500); // 2Hz System Evaluation Frequency (runs safety logic exactly twice every second)
}

/**
 * Executes system-wide isolation and alarm procedures when a critical fault condition is reached.
 */
void triggerEmergencyTrip(float vib, float temp) {
  systemFault = true;                 // Lock the system into an active alert state
  digitalWrite(RELAY_TRIP_PIN, HIGH); // Physically trip the machinery isolation contactor breaker
  digitalWrite(SYSTEM_LED, LOW);       // Terminate the healthy system green LED indicator
  tone(BUZZER_PIN, 1000);             // Fire continuous high-frequency alert sound over buzzer

  // Output formal technical alert log data down the telemetry port
  Serial.println("\n=== [CRITICAL INTERRUPT: ISOLATION FAULT DETECTED] ===");
  Serial.print("Vibration Delta: "); Serial.print(vib); Serial.println(" m/s^2");
  Serial.print("Core Temperature: "); Serial.print(temp); Serial.println(" C");
  Serial.println("System Action: Emergency Load Shedding & Protective Breaker Trip Active.");

  // Flash warning parameters to localized physical visual matrix dashboard
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!! SYS TRIPPED !!");
  lcd.setCursor(0, 1);
  if (temp > TEMP_MAX_THRESHOLD) lcd.print("Over-Temperature");
  else lcd.print("Excess Vibration");
}

/**
 * Restores normal operating state parameters once physical hazards are verified clear.
 */
void clearEmergencyState() {
  systemFault = false;                // Set system state back to safe status flag
  digitalWrite(RELAY_TRIP_PIN, LOW);  // Re-engage the industrial machinery power contactor
  digitalWrite(SYSTEM_LED, HIGH);     // Re-engage the green healthy status indicator
  noTone(BUZZER_PIN);                 // Deactivate the audial alarm system
  Serial.println("\n>>> Alert Recovery: System parameters normalized. Continuous monitoring resumed.");
}

/**
 * Compiles and formats streaming system operational statistics for local operator consumption.
 */
void updateNormalTelemetry(float vib, float temp) {
  // Transmit cleanly formatted engineering data values down the UART/Serial port line
  Serial.print("Telemetry Stream -> Temp: ");
  Serial.print(temp);
  Serial.print("C | Vibration Vector Delta: ");
  Serial.print(vib);
  Serial.println(" m/s^2 [Status: Stable/Nominal]");

  // Refresh physical LCD alphanumeric matrix dashboard interface
  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print(temp, 1); lcd.print("C     ");
  lcd.setCursor(0, 1);
  lcd.print("Vib: "); lcd.print(vib, 2); lcd.print(" m/s2  ");
}