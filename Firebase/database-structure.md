# ☁️ Firebase Realtime Database Structure

This document describes the Firebase Realtime Database structure implemented by the ESP32 firmware.

The ESP32 collects GPS data, displays the information on an SH1106 OLED, and uploads tracking data to Firebase Realtime Database over Wi-Fi.

---

## 🌳 Database Structure

The firmware writes data to exactly three locations:

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
│   └── <Firebase-generated-key>
│       ├── lat
│       ├── lng
│       ├── speed
│       ├── sats
│       └── timestamp
│
└── status
    └── heartbeat
```

> The firmware does not create any other Firebase database paths.

---

# 📍 `/gps` — Live Vehicle Data

The `/gps` node stores the **latest valid GPS information** of the vehicle.

The firmware updates this node using:

```cpp
Firebase.RTDB.setJSON(
    &firebaseData,
    "/gps",
    &json
);
```

Because `setJSON()` is used, the previous `/gps` object is replaced with the latest object.

### Structure

```text
/gps
├── lat
├── lng
├── speed
├── sats
└── timestamp
```

### Example

```json
{
  "lat": 23.810331,
  "lng": 90.412521,
  "speed": 25.4,
  "sats": 8,
  "timestamp": {
    ".sv": "timestamp"
  }
}
```

Firebase resolves the server timestamp when the data is written.

### Fields

| Field       | Type             | Source in Code           | Description                    |
| ----------- | ---------------- | ------------------------ | ------------------------------ |
| `lat`       | Float            | `gps.location.lat()`     | Current latitude               |
| `lng`       | Float            | `gps.location.lng()`     | Current longitude              |
| `speed`     | Float            | `filteredSpeed`          | Filtered speed in km/h         |
| `sats`      | Integer          | `gps.satellites.value()` | Number of satellites           |
| `timestamp` | Server timestamp | `timestamp/.sv`          | Firebase server-side timestamp |

---

# 🧭 `/history` — Vehicle Movement History

The `/history` node stores GPS records while the vehicle is moving.

The firmware uses:

```cpp
Firebase.RTDB.pushJSON(
    &firebaseData,
    "/history",
    &json
);
```

Because `pushJSON()` is used, Firebase automatically generates a unique child key for every record.

### Structure

```text
/history
├── <Firebase-generated-key-1>
│   ├── lat
│   ├── lng
│   ├── speed
│   ├── sats
│   └── timestamp
│
├── <Firebase-generated-key-2>
│   ├── lat
│   ├── lng
│   ├── speed
│   ├── sats
│   └── timestamp
│
└── ...
```

### Example

```json
{
  "history": {
    "-OExampleKey001": {
      "lat": 23.810331,
      "lng": 90.412521,
      "speed": 25.4,
      "sats": 8,
      "timestamp": {
        ".sv": "timestamp"
      }
    }
  }
}
```

The actual Firebase-generated keys will be different.

---

# 🚗 When Is History Saved?

The firmware only pushes a history record when:

```cpp
if (filteredSpeed > 0.0)
```

Therefore:

```text
GPS Valid
    │
    ▼
Speed Filtering
    │
    ▼
filteredSpeed > 0 ?
    │
 ┌──┴──┐
 │     │
YES    NO
 │     │
 ▼     ▼
Push   Do not
History save history
```

The speed is filtered first:

```cpp
if (rawSpeed < 3.0) {
    filteredSpeed = 0.0;
}
```

Therefore, GPS speeds below `3.0 km/h` are treated as `0 km/h`, and those readings are **not added to `/history`**.

---

# ❤️ `/status/heartbeat` — Device Heartbeat

The firmware updates:

```text
/status/heartbeat
```

using:

```cpp
Firebase.RTDB.setInt(
    &firebaseData,
    "/status/heartbeat",
    millis() / 1000
);
```

### Structure

```text
/status
└── heartbeat
```

### Example

```json
{
  "status": {
    "heartbeat": 245
  }
}
```

The value represents:

```cpp
millis() / 1000
```

which is the approximate number of seconds since the ESP32 started.

### Important

This is **not a Firebase server timestamp**.

It is the ESP32's own uptime in seconds.

---

# 📦 JSON Object Created by the Firmware

The firmware creates one `FirebaseJson` object:

```cpp
json.clear();

json.add("lat", latitude);
json.add("lng", longitude);
json.add("speed", filteredSpeed);
json.add("sats", satellitesCount);

json.set(
    "timestamp/.sv",
    "timestamp"
);
```

So the GPS JSON structure is:

```text
{
    lat
    lng
    speed
    sats
    timestamp
}
```

This same JSON object is used for both:

```text
/gps
```

and:

```text
/history/<generated-key>
```

---

# ⏱️ Firebase Update Timing

The firmware defines:

```cpp
const unsigned long UPDATE_INTERVAL = 2000;
```

Therefore, Firebase operations are attempted approximately every **2 seconds**.

However, Firebase updates do **not** occur simply because 2 seconds have passed.

The following conditions must be satisfied first:

```text
GPS location valid
        │
        ▼
Satellite count >= 4
        │
        ▼
Latitude/Longitude != 0
        │
        ▼
2-second interval reached
        │
        ▼
Firebase.ready()
        │
        ▼
Firebase operations
```

---

# 📡 Firebase Operations

During each Firebase update cycle, the firmware performs these operations:

### 1. History Upload

Only when:

```cpp
filteredSpeed > 0.0
```

Operation:

```cpp
Firebase.RTDB.pushJSON(
    &firebaseData,
    "/history",
    &json
);
```

---

### 2. Live GPS Upload

The latest GPS JSON is written to:

```text
/gps
```

using:

```cpp
Firebase.RTDB.setJSON(
    &firebaseData,
    "/gps",
    &json
);
```

This means `/gps` always represents the latest uploaded GPS state.

---

### 3. Heartbeat Update

The ESP32 writes its uptime to:

```text
/status/heartbeat
```

using:

```cpp
Firebase.RTDB.setInt(
    &firebaseData,
    "/status/heartbeat",
    millis() / 1000
);
```

---

# 🔄 Complete Firebase Data Flow

```text
                  🛰️ GPS MODULE
                       │
                       ▼
                GPS NMEA Data
                       │
                       ▼
                  ⚡ ESP32
                       │
                       ▼
                GPS Validation
                       │
              ┌────────┴────────┐
              │                 │
          Invalid             Valid
              │                 │
              ▼                 ▼
       Searching GPS       Speed Filtering
                                │
                                ▼
                         Create FirebaseJson
                                │
                                ▼
                         Firebase.ready()?
                                │
                         ┌──────┴──────┐
                         │             │
                        YES            NO
                         │             │
                         ▼             ▼
                  Firebase Update    Skip
                         │
          ┌──────────────┼──────────────┐
          │              │              │
          ▼              ▼              ▼
       /history         /gps      /status/heartbeat
          │              │              │
    If moving       Latest data       ESP32 uptime
```

---

# 🧱 Actual Firebase Tree

After the device has been running, the database can look like:

```text
ROOT
│
├── gps
│   ├── lat: 23.810331
│   ├── lng: 90.412521
│   ├── speed: 25.4
│   ├── sats: 8
│   └── timestamp: <Firebase server timestamp>
│
├── history
│   │
│   ├── -OABC123...
│   │   ├── lat: 23.810300
│   │   ├── lng: 90.412400
│   │   ├── speed: 21.8
│   │   ├── sats: 7
│   │   └── timestamp: <server timestamp>
│   │
│   ├── -ODEF456...
│   │   ├── lat: 23.810331
│   │   ├── lng: 90.412521
│   │   ├── speed: 25.4
│   │   ├── sats: 8
│   │   └── timestamp: <server timestamp>
│   │
│   └── ...
│
└── status
    └── heartbeat: 245
```

---

# 🔐 Authentication

The firmware initializes Firebase using the configured API key and Realtime Database URL.

It attempts anonymous authentication with:

```cpp
Firebase.signUp(
    &config,
    &auth,
    "",
    ""
);
```

Anonymous authentication must be enabled in the Firebase project for this approach to work.

---

# 🔑 Configuration

The firmware requires:

```cpp
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
#define API_KEY "YOUR_FIREBASE_API_KEY"
```

For Wi-Fi:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```

### ⚠️ Security

Do not commit real Wi-Fi passwords or private configuration values to a public GitHub repository.

Use placeholders in the public source code:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define DATABASE_URL "YOUR_FIREBASE_DATABASE_URL"
#define API_KEY "YOUR_FIREBASE_API_KEY"
```

---

# 📌 Firebase Path Summary

| Firebase Path       | Method       | Purpose          | Condition                        |
| ------------------- | ------------ | ---------------- | -------------------------------- |
| `/gps`              | `setJSON()`  | Latest GPS data  | Firebase ready + valid GPS       |
| `/history`          | `pushJSON()` | Movement history | Firebase ready + vehicle moving  |
| `/status/heartbeat` | `setInt()`   | ESP32 uptime     | Firebase ready + update interval |

---

# ✅ Firmware-to-Database Mapping

```text
GPS
│
├── gps.location.lat()
│       └──────────────► /gps/lat
│                         /history/<key>/lat
│
├── gps.location.lng()
│       └──────────────► /gps/lng
│                         /history/<key>/lng
│
├── filteredSpeed
│       └──────────────► /gps/speed
│                         /history/<key>/speed
│
├── gps.satellites.value()
│       └──────────────► /gps/sats
│                         /history/<key>/sats
│
└── Firebase Server Timestamp
        └──────────────► /gps/timestamp
                          /history/<key>/timestamp


millis() / 1000
        └──────────────► /status/heartbeat
```

---

## 📝 Summary

The current firmware uses a simple three-part Firebase structure:

```text
/gps
```

Stores the **latest uploaded GPS state**.

```text
/history
```

Stores **individual movement records with Firebase-generated keys**.

```text
/status/heartbeat
```

Stores the **ESP32 uptime value** used as a basic device heartbeat.

This documentation directly reflects the Firebase paths and write operations implemented in the current ESP32 firmware.
