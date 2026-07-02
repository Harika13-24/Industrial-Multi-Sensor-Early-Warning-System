# Industrial-Multi-Sensor-Early-Warning-System
Industrial Multi-Sensor Early Warning &amp; Protection System using ESP32, MPU6050, and DHT22 simulated on Wokwi.

# Industrial Multi-Sensor Early Warning & Protection System

## Project Overview
The Industrial Multi-Sensor Early Warning & Protection System is an ESP32-based predictive safety system designed for industrial equipment such as motors, turbines, and boilers. The system continuously monitors vibration and temperature parameters and automatically triggers protective actions whenever abnormal conditions are detected.

## Objectives
- Monitor machine temperature in real time.
- Detect abnormal vibrations.
- Generate audio and visual alarms.
- Trip industrial loads using a relay module.
- Display live sensor readings on an LCD.

## Components Used
1. ESP32 Development Board
2. MPU6050 Accelerometer and Gyroscope Sensor
3. DHT22 Temperature Sensor
4. Relay Module
5. Active Buzzer
6. 16x2 I2C LCD Display
7. LED Indicators
8. Jumper Wires and Breadboard

## Working Principle
1. ESP32 initializes all sensors and peripherals.
2. The MPU6050 measures vibration levels.
3. The DHT22 measures ambient temperature.
4. Sensor data is compared with predefined threshold values:
   - Temperature Threshold: 60°C
   - Vibration Threshold: 2.50 m/s²
5. If any threshold is exceeded:
   - Relay turns ON to isolate the equipment.
   - Buzzer generates an alarm.
   - LCD displays fault information.
6. When parameters return to normal, the system automatically resumes monitoring.

## Features
- Real-time monitoring
- Automatic fault detection
- Predictive maintenance support
- Emergency load isolation
- LCD-based live telemetry
- Audible and visual warning system

## Applications
- Industrial motor protection
- Boiler monitoring systems
- Turbine health monitoring
- Predictive maintenance systems
- Smart factory automation

## Software Used
- Wokwi Simulator
- Arduino IDE
- ESP32 Board Package
- Adafruit MPU6050 Library
- DHT Sensor Library
- LiquidCrystal I2C Library

## Dataset
The project generates sensor readings containing:
- Timestamp
- Temperature (°C)
- Vibration (m/s²)
- System Status (Normal/Fault)
- Relay Status (ON/OFF)
- Buzzer Status (ON/OFF)

Example:

| Timestamp | Temperature (°C) | Vibration (m/s²) | Status | Relay | Buzzer |
|-----------|------------------|------------------|---------|--------|---------|
| 2026-07-02 10:00:00 | 32.5 | 0.45 | Normal | OFF | OFF |
| 2026-07-02 10:02:00 | 42.7 | 2.65 | Fault | ON | ON |

## Folder Structure
Industrial-MultiSensor-EarlyWarning-System/
│
├── sketch.ino
├── diagram.json
├── libraries.txt
├── dataset.csv
├── README.md
└── images/
├── circuit_diagram.png
└── system_output.png

## Future Enhancements
- IoT cloud monitoring
- Mobile application integration
- Email and SMS alerts
- Data logging using an SD card
- Machine learning-based fault prediction

## Authors
Harika. Nenavath  
 B.Tech (23UEE137)   
Department of Electrical Engineering  
National Institute of Technology Agartala
