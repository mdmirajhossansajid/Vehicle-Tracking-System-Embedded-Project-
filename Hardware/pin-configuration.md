# 🔌 Hardware Pin Configuration

This document describes the hardware connections used in the **Vehicle Tracking System**.

## 🧩 Hardware Components

* ESP32 Development Board
* Ai-Thinker GP-02 GNSS/GPS Module
* 1.3-inch SH1106 OLED Display
* Jumper Wires
* USB Power / Development Cable

---

## 🛰️ GPS/GNSS Module — UART2

The GPS module communicates with the ESP32 using the ESP32's second hardware serial interface (`Serial2`).

| GPS Module | ESP32          | Description      |
| ---------- | -------------- | ---------------- |
| TX         | GPIO 16 (RXD2) | GPS data → ESP32 |
| RX         | GPIO 17 (TXD2) | ESP32 → GPS      |
| VCC        | 3.3V           | Power            |
| GND        | GND            | Ground           |

### Communication Settings

```text
Interface : UART
Baud Rate : 9600
Serial Port: Serial2
```

The ESP32 receives NMEA GPS data from the GNSS module and processes it using the **TinyGPS++** library.

---

## 📺 SH1106 OLED — I²C

The OLED display communicates with the ESP32 through the I²C interface.

| OLED | ESP32   | Description |
| ---- | ------- | ----------- |
| SDA  | GPIO 21 | I²C Data    |
| SCL  | GPIO 22 | I²C Clock   |
| VCC  | 3.3V    | Power       |
| GND  | GND     | Ground      |

### Display Configuration

```text
Controller : SH1106
Resolution : 128 × 64
Interface  : I²C
I²C Address: 0x3C
```

---

## 🔗 Complete Pin Mapping

```text
                 ESP32
            ┌──────────────┐
            │              │
    GPS TX ─┤ GPIO 16      │
    GPS RX ─┤ GPIO 17      │
            │              │
   OLED SDA ┤ GPIO 21      │
   OLED SCL ┤ GPIO 22      │
            │              │
      VCC ──┤ 3.3V         │
      GND ──┤ GND          │
            │              │
            └──────────────┘
                 │
        ┌────────┴─────────┐
        │                  │
   🛰️ GP-02 GPS        📺 SH1106 OLED
```

---

## 📡 Communication Interfaces

| Device      | Interface | ESP32 Pins       |
| ----------- | --------- | ---------------- |
| GP-02 GNSS  | UART      | GPIO 16, GPIO 17 |
| SH1106 OLED | I²C       | GPIO 21, GPIO 22 |
| Wi-Fi       | Built-in  | ESP32 Internal   |

---

## ⚠️ Important Notes

1. GPS communication uses **UART2** through `SerialGPS`.
2. GPS baud rate is configured to **9600 baud**.
3. OLED uses the **I²C address `0x3C`**.
4. OLED resolution is **128×64 pixels**.
5. GPIO 16 is configured as GPS RX.
6. GPIO 17 is configured as GPS TX.
7. GPIO 21 is used for I²C SDA.
8. GPIO 22 is used for I²C SCL.
9. Make sure all devices share a common **GND**.
10. Verify the voltage requirements of the specific hardware modules before powering them.

---

## 💻 Related Code Configuration

The corresponding Arduino configuration is:

```cpp
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_ADDRESS 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
```

This configuration allows the ESP32 to simultaneously receive GPS data through UART and display real-time tracking information through the OLED using I²C.
