# 🚗 Vehicle Tracking System

<div align="center">

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:0f172a,100:2563eb&height=220&section=header&text=Vehicle%20Tracking%20System&fontSize=42&fontColor=ffffff&animation=fadeIn&fontAlignY=38&desc=ESP32%20%7C%20GNSS%20%7C%20OLED%20%7C%20Firebase&descAlignY=58&descSize=18" width="100%"/>

### 🚘 Real-Time IoT Vehicle Monitoring System

**ESP32 • GPS/GNSS • OLED • Wi-Fi • Firebase Realtime Database**

<br>

<img src="https://img.shields.io/badge/Platform-ESP32-000000?style=for-the-badge&logo=espressif&logoColor=white"/>
<img src="https://img.shields.io/badge/Language-C%2FC%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white"/>
<img src="https://img.shields.io/badge/GPS-GNSS-1E88E5?style=for-the-badge"/>
<img src="https://img.shields.io/badge/Firebase-Realtime%20Database-FFCA28?style=for-the-badge&logo=firebase&logoColor=black"/>
<img src="https://img.shields.io/badge/OLED-SH1106-111827?style=for-the-badge"/>

</div>

---

## 🌟 Overview

**Vehicle Tracking System** is an IoT-based embedded tracking solution designed to monitor a vehicle's position and movement in real time.

The system uses an **ESP32** as the central controller. A **GNSS/GPS module** provides location and speed information, while an **SH1106 OLED display** provides real-time local feedback.

Through Wi-Fi, the ESP32 communicates with **Firebase Realtime Database**, where live GPS information, movement history, and system heartbeat data are stored.

### 🎯 Core Capabilities

| Feature                  | Description                          |
| ------------------------ | ------------------------------------ |
| 📍 Live Location         | Real-time latitude and longitude     |
| 🚗 Speed Monitoring      | GPS-based vehicle speed              |
| 🛰️ Satellite Monitoring | Current GPS satellite count          |
| 📺 OLED Dashboard        | Local real-time vehicle status       |
| ☁️ Cloud Storage         | Firebase Realtime Database           |
| 🧭 Movement History      | Stores location history while moving |
| ❤️ Heartbeat             | Device communication monitoring      |
| 🔄 Auto Reconnection     | Firebase network reconnection        |

---

# 🧠 System Architecture

```mermaid
flowchart LR

    GPS["🛰️ GNSS / GPS Module"]
    ESP["⚡ ESP32 Controller"]
    OLED["📺 SH1106 OLED"]
    WIFI["📡 Wi-Fi Network"]
    FB["☁️ Firebase Realtime Database"]

    GPS -->|"Latitude / Longitude\nSpeed / Satellites"| ESP

    ESP -->|"Live Status"| OLED

    ESP -->|"HTTPS / Internet"| WIFI
    WIFI --> FB

    FB --> LIVE["📍 /gps\nLive Location"]
    FB --> HISTORY["🧭 /history\nMovement History"]
    FB --> STATUS["❤️ /status\nHeartbeat"]

    style ESP fill:#2563eb,color:#fff,stroke:#1e3a8a
    style GPS fill:#111827,color:#fff,stroke:#374151
    style OLED fill:#111827,color:#fff,stroke:#374151
    style WIFI fill:#0f766e,color:#fff,stroke:#134e4a
    style FB fill:#f59e0b,color:#111827,stroke:#92400e
```

---

# 🔄 Real-Time Data Flow

```mermaid
sequenceDiagram

    participant G as 🛰️ GPS
    participant E as ⚡ ESP32
    participant O as 📺 OLED
    participant W as 📡 Wi-Fi
    participant F as ☁️ Firebase

    G->>E: GPS NMEA Data
    E->>E: Parse GPS Data
    E->>E: Validate Location
    E->>E: Filter Speed

    E->>O: Display Speed
    E->>O: Display Coordinates
    E->>O: Display Satellites

    E->>W: Send Data
    W->>F: Upload JSON

    F->>F: Update /gps

    alt Vehicle Moving
        F->>F: Push /history
    end

    F->>F: Update Heartbeat
```

---

# 🔌 Hardware Architecture

```mermaid
flowchart TB

    POWER["🔋 Power Supply"]

    ESP["⚡ ESP32"]

    GPS["🛰️ Ai-Thinker GP-02\nGNSS Module"]
    OLED["📺 1.3-inch SH1106\nOLED Display"]

    POWER --> ESP

    GPS -->|"TX → GPIO 16"| ESP
    ESP -->|"GPIO 17 → RX"| GPS

    ESP -->|"I²C SDA → GPIO 21\nI²C SCL → GPIO 22"| OLED

    style ESP fill:#2563eb,color:#fff,stroke:#1e40af
    style GPS fill:#111827,color:#fff
    style OLED fill:#111827,color:#fff
    style POWER fill:#374151,color:#fff
```

---

# 📡 Communication Architecture

```text
┌─────────────────────────────────────────────────────────────┐
│                     VEHICLE SYSTEM                         │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   🛰️ GPS/GNSS                                               │
│        │                                                    │
│        │ UART                                               │
│        ▼                                                    │
│   ⚡ ESP32 ──────────────── I²C ───────────────► 📺 OLED   │
│        │                                                    │
│        │ Wi-Fi                                              │
│        ▼                                                    │
│   📡 Internet                                                │
│        │                                                    │
│        ▼                                                    │
│   ☁️ Firebase Realtime Database                             │
│        │                                                    │
│        ├──────────► 📍 Live GPS                             │
│        │                                                    │
│        ├──────────► 🧭 Movement History                     │
│        │                                                    │
│        └──────────► ❤️ Device Heartbeat                     │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

# 📊 Firebase Data Architecture

The system uses a simple and scalable Firebase structure:

```text
/
├── gps
│   ├── lat
│   ├── lng
│   ├── speed
│   ├── sats
│   └── timestamp
│
├── history
│   ├── record_001
│   │   ├── lat
│   │   ├── lng
│   │   ├── speed
│   │   ├── sats
│   │   └── timestamp
│   │
│   ├── record_002
│   └── ...
│
└── status
    └── heartbeat
```

### Example Live GPS Object

```json
{
  "lat": 23.810331,
  "lng": 90.412521,
  "speed": 25.4,
  "sats": 8,
  "timestamp": 1727000000000
}
```

---

# 📺 OLED Interface

The OLED provides a compact real-time dashboard.

### GPS Searching

```text
┌────────────────────────┐
│ VEHICLE TRACKING       │
│────────────────────────│
│ Searching GPS...       │
│ Waiting for fix        │
│                        │
│ Satellites: 3         │
└────────────────────────┘
```

### GPS Fixed

```text
┌────────────────────────┐
│      25.4 km/h         │
│────────────────────────│
│ Lat: 23.810331         │
│ Lng: 90.412521         │
│ Sats: 8                │
└────────────────────────┘
```

---

# 🧠 Intelligent GPS Processing

The ESP32 does not blindly upload every GPS reading.

The system performs several processing steps:

```mermaid
flowchart LR

    RAW["Raw GPS Data"]
    VALID{"GPS Valid?"}
    SAT{"≥ 4 Satellites?"}
    SPEED["Speed Filtering"]
    OLED["OLED Display"]
    CLOUD["Firebase"]

    RAW --> VALID

    VALID -->|No| WAIT["Searching GPS"]
    VALID -->|Yes| SAT

    SAT -->|No| WAIT
    SAT -->|Yes| SPEED

    SPEED --> OLED
    SPEED --> CLOUD

    style RAW fill:#111827,color:#fff
    style VALID fill:#7c3aed,color:#fff
    style SAT fill:#7c3aed,color:#fff
    style SPEED fill:#2563eb,color:#fff
    style OLED fill:#0f766e,color:#fff
    style CLOUD fill:#f59e0b,color:#111827
```

---

# 🚗 Speed Filtering

GPS modules can sometimes report very small speeds even when the vehicle is stationary.

To reduce unnecessary movement detection:

```cpp
if (rawSpeed < 3.0) {
    filteredSpeed = 0.0;
}
```

Therefore:

```text
Raw Speed < 3 km/h
        ↓
Considered Stationary
        ↓
Filtered Speed = 0 km/h
```

This also prevents unnecessary history records when the vehicle is effectively stopped.

---

# ⏱️ Cloud Update Strategy

The ESP32 updates Firebase every **2 seconds**.

```text
GPS Reading
     ↓
Process Data
     ↓
OLED Update
     ↓
2 Second Interval
     ↓
Firebase Update
     ├── /gps
     ├── /history   → Only when moving
     └── /status/heartbeat
```

---

# 🔌 Pin Configuration

### GPS — UART2

| GPS Pin   | ESP32 Pin |
| --------- | --------- |
| TX        | GPIO 16   |
| RX        | GPIO 17   |
| Baud Rate | 9600      |

### OLED — I²C

| OLED Pin | ESP32 Pin |
| -------- | --------- |
| SDA      | GPIO 21   |
| SCL      | GPIO 22   |
| VCC      | 3.3V      |
| GND      | GND       |
| Address  | `0x3C`    |

---

# 🛠️ Technology Stack

<div align="center">

| Layer           | Technology                 |
| --------------- | -------------------------- |
| Microcontroller | ESP32                      |
| Programming     | C / C++                    |
| Positioning     | GNSS / GPS                 |
| Display         | SH1106 OLED                |
| Communication   | UART / I²C                 |
| Network         | Wi-Fi                      |
| Cloud           | Firebase Realtime Database |
| Development     | Arduino IDE                |

</div>

---

# 📚 Libraries

```cpp
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
```

### Required Libraries

* `Firebase ESP Client`
* `TinyGPS++`
* `Adafruit GFX`
* `Adafruit SH110X`
* ESP32 Board Package

---

# 🚀 Installation

## 1. Clone Repository

```bash
git clone https://github.com/mdmirajhossansajid/Vehicle-Tracking-System.git
```

## 2. Open Arduino Project

Open:

```text
ESP32/vehicle_tracking.ino
```

using Arduino IDE.

## 3. Install Dependencies

Install the required libraries through:

```text
Arduino IDE
→ Sketch
→ Include Library
→ Manage Libraries
```

## 4. Configure Wi-Fi

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

## 5. Configure Firebase

```cpp
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
#define API_KEY "YOUR_FIREBASE_API_KEY"
```

## ⚠️ Security

**Never commit real Wi-Fi passwords or private credentials to a public repository.**

For GitHub, keep placeholders such as:

```cpp
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_KEY "YOUR_FIREBASE_API_KEY"
```

---

# 📁 Project Structure

```text
Vehicle-Tracking-System/
│
├── ESP32/
│   └── vehicle_tracking.ino
│
├── Hardware/
│   └── pin-configuration.md
│
├── Firebase/
│   └── database-structure.md
│
├── README.md
│
└── .gitignore
```

---

# ⚙️ Main Functional Modules

```text
┌───────────────────────────────────────────┐
│          VEHICLE TRACKING SYSTEM          │
├───────────────────────────────────────────┤
│                                           │
│  🛰️ GPS Acquisition                       │
│       ↓                                   │
│  🧠 GPS Validation                        │
│       ↓                                   │
│  🚗 Speed Processing                      │
│       ↓                                   │
│  📺 OLED Visualization                    │
│       ↓                                   │
│  📡 Wi-Fi Communication                   │
│       ↓                                   │
│  ☁️ Firebase Cloud Storage                │
│       ↓                                   │
│  🧭 Movement History                      │
│                                           │
└───────────────────────────────────────────┘
```

---

# 🎯 Project Objectives

* Develop a real-time GPS vehicle tracking system.
* Process GNSS data using ESP32.
* Monitor vehicle speed and satellite availability.
* Display live information through an OLED.
* Transmit tracking data through Wi-Fi.
* Store live location data in Firebase.
* Maintain movement history.
* Implement a lightweight embedded IoT monitoring architecture.

---

# 🔮 Future Improvements

The system can be extended with:

* 🗺️ Interactive live map
* 📍 Route visualization
* 📏 Total distance calculation
* 🚨 Overspeed alerts
* 🔔 Geofencing
* 📱 Android/iOS application
* 📊 Trip analytics
* 🔐 Improved authentication
* 🔋 Battery monitoring
* 📡 Offline data buffering
* 🚘 Multiple vehicle support

---

# 🏆 Project Highlights

```text
                ┌──────────────────┐
                │   REAL-TIME GPS  │
                └────────┬─────────┘
                         │
             ┌───────────┴───────────┐
             │                       │
             ▼                       ▼
       📺 LOCAL MONITORING      ☁️ CLOUD STORAGE
             │                       │
             ▼                       ▼
        OLED DISPLAY          FIREBASE RTDB
                                     │
                         ┌───────────┴───────────┐
                         │                       │
                         ▼                       ▼
                    📍 LIVE DATA          🧭 HISTORY
```

---

# 👨‍💻 Author

### Md. Miraj Hossan Sajid

**Computer Science & Engineering**
**Southeast University**

**GitHub:** `mdmirajhossansajid`
**LinkedIn:** `mdmirajhossansajid`
**Codeforces:** `Paradoxical_Sajid`

---

<div align="center">

### 🚗 Built with ESP32 • GPS • Firebase • C/C++

**Real-Time Tracking • Embedded IoT • Cloud Connectivity**

<img src="https://capsule-render.vercel.app/api?type=waving&color=0:2563eb,100:0f172a&height=120&section=footer" width="100%"/>

</div>
