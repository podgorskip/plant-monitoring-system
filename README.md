# Smart pot - intelligent plant monitoring system

## Overview

The Smart pot is an intelligent plant monitoring system based on the ESP32 module. It automates key plant care processes using modern technology, eliminating the need for constant user supervision. The system monitors essential environmental parameters and provides real-time updates via MQTT and Bluetooth Low Energy (BLE).

## Features

- **Soil moisture monitoring**: prevents overwatering and underwatering.
- **Air humidity & temperature measurement**: helps maintain optimal plant conditions.
- **Sunlight intensity detection**: ensures proper light exposure for photosynthesis.
- **Water level monitoring**: alerts when water needs replenishment.
- **Automated watering system**: uses a relay-controlled pump.
- **BLE-based configuration**: enables easy setup via the nRF Connect app.
- **MQTT communication**: provides real-time monitoring and remote control.
- **PostgreSQL database**: stores sensor data, user configurations, and logs.

## Hardware components

- **ESP32 Microcontroller**: central processing unit.
- **Capacitive Soil Moisture Sensor (V1.2)**: measures soil moisture percentage.
- **BME280 Sensor**: reads air humidity and temperature via I2C.
- **LM393 LDR Sensor**: measures light intensity (analog/digital readings).
- **Iduino SE045 Water Level Sensor**: monitors water reservoir levels.
- **120L/h Water Pump**: used for automated plant watering.
- **Relay Module (SRD-05VDC-SL-C)**: controls the water pump.
- **10,000mAh Power Bank**: provides stable power supply.

## Software architecture

- **Programming languages**: C (ESP-IDF for ESP32), Java (Backend), Angular (Frontend).
- **Communication protocols**:
  - **BLE**: initial configuration (Wi-Fi credentials, MQTT broker settings).
  - **MQTT**: data transmission and remote commands.
- **Data format**: JSON over MQTT.
- **Database**: PostgreSQL for sensor data storage and user management.

## PostgreSQL database

The system uses PostgreSQL for efficient data storage and retrieval. The database schema includes:

- **Users**: stores user authentication details.
- **Devices**: stores registered Smart Pot devices.
- **Sensor data**: stores time-series data of soil humidity, air humidity, temperature, and light intensity.
- **Watering logs**: tracks automated and manual watering actions.
- **Notifications**: logs alert messages sent via Twilio.

### Example database table structure

```sql
DROP TABLE IF EXISTS user_details;

CREATE TABLE user_details (
    id BIGINT PRIMARY KEY,
    mac CHAR(18) NOT NULL UNIQUE,
    first_name VARCHAR(255) NOT NULL,
    last_name VARCHAR(255) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    phone_number CHAR(9) NOT NULL UNIQUE,
    password TEXT NOT NULL,
    role VARCHAR(20) NOT NULL,
    verified BOOLEAN NOT NULL DEFAULT false
);

DROP TABLE IF EXISTS air_humidity;

CREATE TABLE air_humidity (
    id BIGINT PRIMARY KEY,
    value NUMERIC(5, 2) NOT NULL,
    device_id BIGINT NOT NULL REFERENCES device(id),
    date TIMESTAMP NOT NULL DEFAULT NOW()
);
```

## BLE configuration workflow

1. Press the **boot** button to enter BLE configuration mode.
2. Connect via the nRF Connect app.
3. Configure Wi-Fi SSID, password, and MQTT broker details.
4. Save settings (stored in NVS memory).
5. Device connects to Wi-Fi and MQTT server.

## MQTT communication

### Topics for sensor data:

```
/user_mac/device_mac/air_humidity
/user_mac/device_mac/soil_humidity
/user_mac/device_mac/temperature
/user_mac/device_mac/insolation
/user_mac/device_mac/insolation_digital
```

Example message:

```json
{
  "value": 45.2,
  "timestamp": "1736679460",
  "unit": "%"
}
```

### Topics for commands:

```
/user_mac/device_mac/soil_humidity/request  # Request watering duration (in seconds)
```

### Topics for configuration:

```
/user_mac/device_mac/{parameter}/frequency  # Request sensor data read (in seconds)
```

where *parameter* is one of: *air\_humidity/soil\_humidity/temperature/insolation/insolation\_digital*.

## Server application (backend & frontend)

- **Backend (Spring Boot)**:
  - Handles authentication and user management.
  - Stores sensor data in PostgreSQL.
  - Provides an API for real-time monitoring and control.
- **Frontend (Angular)**:
  - **Registration panel**: user account setup.
  - **Login panel**: secure authentication.
  - **Device dashboard**: view and manage connected devices.
    
      <img width="1454" alt="measurements" src="https://github.com/user-attachments/assets/73883e48-4487-4010-a754-1da3acf8da68" />
      
  - **Manual watering control**: initiate watering from the app.
    
    <img width="1453" alt="watering" src="https://github.com/user-attachments/assets/56676f24-b0e3-4798-bd2d-5ae8218f13a4" />
    
  - **Threshold configuration**: set min/max limits for alerts.
    
      <img width="1454" alt="thresholds" src="https://github.com/user-attachments/assets/bc0c69a2-16ab-42ff-a4a2-44917ece6be8" />

  - **Notifications**: SMS alerts via Twilio.

## Docker-based deployment

The system can be deployed using Docker with the following services:

- **Backend (Spring Boot application)**
- **Frontend (Angular application)**
- **Database (PostgreSQL 15)**
- **MQTT Broker (Eclipse Mosquitto)**

## Running the Smart Pot system

### Prerequisites

- Docker & Docker Compose installed
- ESP32 microcontroller with firmware flashed
- MQTT broker configured

### Steps to run

1. Clone the repository:
   ```sh
   git clone https://github.com/podgorskip/smart-pot
   cd smart-pot
   ```

2. Configure environment variables:
   ```sh
   cp .config/app/.env.example .config/app/.env
   cp .config/db/.env.example .config/db/.env
   cp .config/broker/.env.example .config/broker/.env
   ```
   Edit the `.env` files with your settings.

3. Build and start services:
   ```sh
   docker-compose up --build
