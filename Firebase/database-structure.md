# ☁️ Firebase Realtime Database Structure

This document describes the Firebase Realtime Database structure used by the **Vehicle Tracking System**.

The ESP32 sends GPS information to Firebase through Wi-Fi using the **Firebase Realtime Database**.

---

## 🌳 Database Structure

The database contains three main sections:

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
│   ├── -Oxxxxxxxxxxxx
│   │   ├── lat
│   │   ├── lng
│   │   ├── speed
│   │   ├── sats
│   │   └── timestamp
│   │
│   ├── -Oyyyyyyyyyyyy
│   │   └── ...
│   │
│   └── ...
│
└── status
    └── heartbeat
```

---

# 📍 1. Live GPS Data

The current vehicle location is stored at:

```text
/gps
```

The ESP32 updates this location approximately every **2 seconds**.

### Data Structure

```json
{
  "lat": 23.810331,
  "lng": 90.412521,
  "speed": 25.4,
  "sats": 8,
  "timestamp": 1727000000000
}
```

### Fields

| Field       | Type             | Description                    |
| ----------- | ---------------- | ------------------------------ |
| `lat`       | Float            | Current latitude               |
| `lng`       | Float            | Current longitude              |
| `speed`     | Float            | Vehicle speed in km/h          |
| `sats`      | Integer          | Number of connected satellites |
| `timestamp` | Server Timestamp | Firebase server-side timestamp |

### Example

```text
/gps
    ├── lat       → 23.810331
    ├── lng       → 90.412521
    ├── speed     → 25.4
    ├── sats      → 8
    └── timestamp → Firebase Server Timestamp
```

---

# 🧭 2. Movement History

Historical tracking data is stored at:

```text
/history
```

Unlike `/gps`, which contains only the **latest location**, `/history` contains multiple GPS records.

The ESP32 uses Firebase `pushJSON()` to create a unique key for each history record.

### Example

```text
/history
    │
    ├── -Oabc123
    │     ├── lat
    │     ├── lng
    │     ├── speed
    │     ├── sats
    │     └── timestamp
    │
    ├── -Odef456
    │     ├── lat
    │     ├── lng
    │     ├── speed
    │     ├── sats
    │     └── timestamp
    │
    └── ...
```

### Example History Record

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

# 🚗 History Recording Logic

The system does **not** create a history record when the vehicle is stationary.

The code checks:

```cpp
if (filteredSpeed > 0.0) {
    Firebase.RTDB.pushJSON(
        &firebaseData,
        "/history",
        &json
    );
}
```

Therefore:

```text
                 GPS Data
                    │
                    ▼
              Speed Filtering
                    │
                    ▼
             ┌──────────────┐
             │ Speed > 0 ?  │
             └──────┬───────┘
                    │
              ┌─────┴─────┐
              │           │
             YES          NO
              │           │
              ▼           ▼
        Save History   No History
              │
              ▼
       /history/{key}
```

This reduces unnecessary history entries while the vehicle is stopped.

---

# ❤️ 3. Device Heartbeat

The device heartbeat is stored at:

```text
/status/heartbeat
```

The ESP32 periodically updates this value with its running time:

```cpp
millis() / 1000
```

### Example

```json
{
  "status": {
    "heartbeat": 245
  }
}
```

Here, `245` represents approximately **245 seconds since the ESP32 started**.

The heartbeat can be used to determine whether the device is actively communicating with Firebase.

---

# 🔄 Firebase Update Cycle

The ESP32 follows this process:

```text
                 🛰️ GPS
                   │
                   ▼
             Read GPS Data
                   │
                   ▼
           Validate GPS Fix
                   │
                   ▼
            Filter Speed
                   │
                   ▼
             Create JSON
                   │
          ┌────────┼────────┐
          │        │        │
          ▼        ▼        ▼
        /gps   /history  /status
          │        │        │
          │        │        └── heartbeat
          │        │
          │        └── Only when moving
          │
          └── Current location
```

---

# ⏱️ Update Frequency

The Firebase update interval is configured as:

```cpp
const unsigned long UPDATE_INTERVAL = 2000;
```

Therefore, Firebase updates are attempted approximately every:

```text
2 seconds
```

### Update Behavior

| Data                | Update Condition                        |
| ------------------- | --------------------------------------- |
| `/gps`              | Every 2 seconds when Firebase is ready  |
| `/history`          | Every 2 seconds while vehicle is moving |
| `/status/heartbeat` | Every Firebase update cycle             |

---

# 🧱 JSON Object Used by ESP32

The ESP32 creates a single JSON object:

```cpp
json.clear();

json.add("lat", latitude);
json.add("lng", longitude);
json.add("speed", filteredSpeed);
json.add("sats", satellitesCount);

json.set("timestamp/.sv", "timestamp");
```

This object is then used for both:

```text
/gps
```

and:

```text
/history
```

---

# 🔐 Firebase Authentication

The ESP32 uses Firebase authentication during startup.

The project uses:

```cpp
Firebase.signUp(
    &config,
    &auth,
    "",
    ""
);
```

This enables anonymous Firebase authentication when anonymous authentication is enabled in the Firebase project.

---

# 🔗 Firebase Operations

The project uses the following Firebase operations:

| Operation | Firebase Method | Purpose                     |
| --------- | --------------- | --------------------------- |
| Live GPS  | `setJSON()`     | Replace current `/gps` data |
| History   | `pushJSON()`    | Create a new history record |
| Heartbeat | `setInt()`      | Update device heartbeat     |

### Live GPS

```cpp
Firebase.RTDB.setJSON(
    &firebaseData,
    "/gps",
    &json
);
```

### History

```cpp
Firebase.RTDB.pushJSON(
    &firebaseData,
    "/history",
    &json
);
```

### Heartbeat

```cpp
Firebase.RTDB.setInt(
    &firebaseData,
    "/status/heartbeat",
    millis() / 1000
);
```

---

# 📊 Data Flow Summary

```text
GPS Module
    │
    │ Latitude
    │ Longitude
    │ Speed
    │ Satellites
    ▼
ESP32
    │
    ├──────────────► OLED Display
    │
    ▼
Wi-Fi
    │
    ▼
Firebase Realtime Database
    │
    ├── /gps
    │     └── Current vehicle state
    │
    ├── /history
    │     └── Historical movement records
    │
    └── /status
          └── Device heartbeat
```

---

# ⚠️ Security Considerations

Do not store sensitive credentials directly in a public GitHub repository.

The following values should be kept private:

```cpp
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define API_KEY "YOUR_FIREBASE_API_KEY"
```

For the public repository, use placeholder values or a separate private configuration file.

---

# 📝 Summary

The Firebase database is organized into three logical areas:

```text
/gps
```

Stores the **latest vehicle location and status**.

```text
/history
```

Stores **historical GPS records while the vehicle is moving**.

```text
/status/heartbeat
```

Stores the **device heartbeat for connectivity monitoring**.

This structure keeps live data, historical data, and device status logically separated and makes the system easier to extend in the future.
