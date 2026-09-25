#include <WiFi.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>


// ============================================================
//                  WIFI CONFIGURATION
// ============================================================

// WiFi Network Name
#define WIFI_SSID "technolab"

// WiFi Password
#define WIFI_PASSWORD "12345678"


// ============================================================
//                FIREBASE CONFIGURATION
// ============================================================

// Your Firebase Realtime Database URL
#define DATABASE_URL "https://vehicle-tracking-system-baac1-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase Web API Key
// এখানে তোমার Firebase API Key বসাবে
#define API_KEY "YOUR_FIREBASE_API_KEY"


// ============================================================
//                    GPS PIN CONFIGURATION
// ============================================================

// GPS Module: Ai-Thinker GP-02 / GPS module
//
// GPS TX  ---> ESP32 GPIO 16 (RX2)
// GPS RX  ---> ESP32 GPIO 17 (TX2)
//
// IMPORTANT:
// GPS TX connects to ESP32 RX
// GPS RX connects to ESP32 TX

#define RXD2 16
#define TXD2 17

// GPS Baud Rate
#define GPS_BAUD 9600


// ============================================================
//                    OLED PIN CONFIGURATION
// ============================================================
//
// OLED: 1.3-inch SH1106 I2C OLED
//
// ESP32        OLED
// ----------------------
// GPIO 21  ---> SDA
// GPIO 22  ---> SCL
// 3.3V     ---> VCC
// GND      ---> GND
//
// I2C Address = 0x3C
//

#define I2C_SDA 21
#define I2C_SCL 22

#define I2C_ADDRESS 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// SH1106 OLED usually does not need a physical reset pin
#define OLED_RESET -1


// ============================================================
//                    OBJECT DECLARATION
// ============================================================

// GPS object
TinyGPSPlus gps;

// ESP32 Hardware Serial 2
HardwareSerial SerialGPS(2);

// OLED object
Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// Firebase objects
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// Firebase JSON object
FirebaseJson json;


// ============================================================
//                    GLOBAL VARIABLES
// ============================================================

// Stores the last Firebase update time
unsigned long lastUpdateTime = 0;

// Firebase update interval
// 2000 milliseconds = 2 seconds
const unsigned long UPDATE_INTERVAL = 2000;


// ============================================================
//                         SETUP
// ============================================================

void setup() {

  // ----------------------------------------------------------
  // Serial Monitor
  // ESP32 USB Serial
  // ----------------------------------------------------------

  Serial.begin(115200);

  // ----------------------------------------------------------
  // GPS Serial Initialization
  //
  // GPIO 16 = RX2
  // GPIO 17 = TX2
  // Baud Rate = 9600
  // ----------------------------------------------------------

  SerialGPS.begin(
    GPS_BAUD,
    SERIAL_8N1,
    RXD2,
    TXD2
  );


  // ----------------------------------------------------------
  // OLED I2C Initialization
  //
  // GPIO 21 = SDA
  // GPIO 22 = SCL
  // ----------------------------------------------------------

  Wire.begin(I2C_SDA, I2C_SCL);

  display.begin(I2C_ADDRESS, true);

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");
  display.println("System Starting...");

  display.display();

  delay(1500);


  // ----------------------------------------------------------
  // WIFI CONNECTION
  // ----------------------------------------------------------

  display.clearDisplay();

  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");
  display.println("WiFi Connecting...");

  display.display();


  Serial.println();
  Serial.println("================================");
  Serial.println("   VEHICLE TRACKING SYSTEM");
  Serial.println("================================");

  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


  int attempts = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    attempts < 30
  ) {

    delay(500);

    Serial.print(".");

    attempts++;

    display.clearDisplay();

    display.setCursor(0, 5);

    display.println("VEHICLE TRACKING");
    display.println("--------------------");

    display.print("WiFi Connecting");

    for (int i = 0; i < attempts % 4; i++) {
      display.print(".");
    }

    display.display();
  }


  // ----------------------------------------------------------
  // CHECK WIFI STATUS
  // ----------------------------------------------------------

  display.clearDisplay();

  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");


  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();
    Serial.println("WiFi Connected!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    display.println("WiFi: CONNECTED");

    display.print("IP: ");
    display.println(WiFi.localIP());

  } else {

    Serial.println();
    Serial.println("WiFi Connection Failed!");

    display.println("WiFi: FAILED");
  }


  display.display();

  delay(1500);


  // ==========================================================
  //                 FIREBASE CONFIGURATION
  // ==========================================================

  config.api_key = API_KEY;

  config.database_url = DATABASE_URL;


  // ----------------------------------------------------------
  // Firebase Authentication
  //
  // This setup uses Firebase API key configuration.
  // If your Firebase database requires authentication,
  // configure Firebase Authentication accordingly.
  // ----------------------------------------------------------

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);


  Serial.println("Firebase initialized.");

  delay(1000);


  // ----------------------------------------------------------
  // Initial OLED Status
  // ----------------------------------------------------------

  display.clearDisplay();

  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");
  display.println("System Ready!");
  display.println("GPS Searching...");

  display.display();

}


// ============================================================
//                          LOOP
// ============================================================

void loop() {

  // ----------------------------------------------------------
  // Read GPS data
  // ----------------------------------------------------------

  while (SerialGPS.available() > 0) {

    char gpsData = SerialGPS.read();

    gps.encode(gpsData);
  }


  // ----------------------------------------------------------
  // Update complete system
  // ----------------------------------------------------------

  updateSystem();
}


// ============================================================
//                    UPDATE SYSTEM FUNCTION
// ============================================================

void updateSystem() {

  // ----------------------------------------------------------
  // Get satellite count
  // ----------------------------------------------------------

  int satellitesCount = gps.satellites.value();


  // ----------------------------------------------------------
  // Check whether GPS location is valid
  // ----------------------------------------------------------

  bool locationValid = gps.location.isValid();


  // ==========================================================
  //             GPS SEARCHING / NO FIX SCREEN
  // ==========================================================

  if (
    !locationValid ||
    satellitesCount < 4 ||
    (
      gps.location.lat() == 0.0 &&
      gps.location.lng() == 0.0
    )
  ) {

    display.clearDisplay();

    display.setTextSize(1);

    display.setCursor(0, 5);

    display.println("VEHICLE TRACKING");

    display.drawLine(
      0,
      16,
      128,
      16,
      SH110X_WHITE
    );


    display.setCursor(0, 25);

    display.println("Searching GPS...");

    display.println("Waiting for fix");


    display.setCursor(0, 50);

    display.print("Satellites: ");

    display.print(satellitesCount);


    // Small animation
    if ((millis() / 1000) % 2 == 0) {
      display.print(" .");
    }


    display.display();


    // Do not continue until GPS fix is available
    return;
  }


  // ==========================================================
  //                  VALID GPS DATA
  // ==========================================================


  // Latitude
  float latitude = gps.location.lat();


  // Longitude
  float longitude = gps.location.lng();


  // Raw GPS speed
  float rawSpeed = gps.speed.kmph();


  // ----------------------------------------------------------
  // SPEED FILTER
  //
  // GPS sometimes produces small speed values while the
  // vehicle is actually stopped.
  //
  // Example:
  // 0.7 km/h -> 0 km/h
  // 1.8 km/h -> 0 km/h
  // 2.9 km/h -> 0 km/h
  // 5.0 km/h -> 5.0 km/h
  // ----------------------------------------------------------

  float filteredSpeed = rawSpeed;

  if (rawSpeed < 3.0) {

    filteredSpeed = 0.0;
  }


  // ==========================================================
  //                  OLED LIVE DASHBOARD
  // ==========================================================

  display.clearDisplay();


  // Speed
  display.setTextSize(2);

  display.setCursor(15, 0);

  display.print(filteredSpeed, 1);


  display.setTextSize(1);

  display.print(" km/h");


  // Separator
  display.drawLine(
    0,
    20,
    128,
    20,
    SH110X_WHITE
  );


  // Latitude
  display.setCursor(0, 28);

  display.print("Lat: ");

  display.println(latitude, 6);


  // Longitude
  display.print("Lng: ");

  display.println(longitude, 6);


  // Satellite count
  display.print("Sats: ");

  display.println(satellitesCount);


  display.display();


  // ==========================================================
  //              FIREBASE UPDATE EVERY 2 SEC
  // ==========================================================

  if (
    millis() - lastUpdateTime >= UPDATE_INTERVAL
  ) {

    lastUpdateTime = millis();


    // --------------------------------------------------------
    // Create JSON object
    // --------------------------------------------------------

    json.clear();


    json.add(
      "lat",
      latitude
    );


    json.add(
      "lng",
      longitude
    );


    json.add(
      "speed",
      filteredSpeed
    );


    json.add(
      "sats",
      satellitesCount
    );


    // Firebase server timestamp
    json.set(
      "timestamp/.sv",
      "timestamp"
    );


    // --------------------------------------------------------
    // Serial Monitor
    // --------------------------------------------------------

    Serial.println();
    Serial.println("--------------------------------");

    Serial.println("Firebase Update");

    Serial.print("Latitude : ");
    Serial.println(latitude, 6);

    Serial.print("Longitude: ");
    Serial.println(longitude, 6);

    Serial.print("Speed    : ");
    Serial.print(filteredSpeed, 1);
    Serial.println(" km/h");

    Serial.print("Satellites: ");
    Serial.println(satellitesCount);


    // ========================================================
    //                  HISTORY DATA
    // ========================================================
    //
    // Only save history when vehicle is moving.
    //
    // Firebase:
    //
    // /history
    //      ├── auto-generated ID
    //      │      ├── lat
    //      │      ├── lng
    //      │      ├── speed
    //      │      ├── sats
    //      │      └── timestamp
    //
    // ========================================================

    if (filteredSpeed > 0.0) {

      if (
        Firebase.pushJSON(
          firebaseData,
          "/history",
          json
        )
      ) {

        Serial.println(
          "History data uploaded successfully."
        );

      } else {

        Serial.print(
          "History upload failed: "
        );

        Serial.println(
          firebaseData.errorReason()
        );
      }
    }


    // ========================================================
    //                    LIVE GPS DATA
    // ========================================================
    //
    // /gps always contains the latest location.
    //
    // This can be used by your web dashboard / Leaflet map
    // to display the current vehicle marker.
    //
    // ========================================================

    if (
      Firebase.setJSON(
        firebaseData,
        "/gps",
        json
      )
    ) {

      Serial.println(
        "Live GPS data uploaded successfully."
      );

    } else {

      Serial.print(
        "Live GPS upload failed: "
      );

      Serial.println(
        firebaseData.errorReason()
      );
    }


    // ========================================================
    //                  DEVICE HEARTBEAT
    // ========================================================
    //
    // This value can be used by the dashboard to determine
    // whether the ESP32 is actively communicating.
    //
    // ========================================================

    if (
      Firebase.setInt(
        firebaseData,
        "/status/heartbeat",
        millis() / 1000
      )
    ) {

      Serial.println(
        "Heartbeat updated."
      );

    } else {

      Serial.print(
        "Heartbeat update failed: "
      );

      Serial.println(
        firebaseData.errorReason()
      );
    }


    Serial.println("--------------------------------");
  }
}

🔌 Pin connection

Component| Component Pin| ESP32 Pin
GPS GP-02| TX| GPIO 16 (RX2)
GPS GP-02| RX| GPIO 17 (TX2)
GPS GP-02| VCC| 3.3V / module-supported supply
GPS GP-02| GND| GND
SH1106 OLED| SDA| GPIO 21
SH1106 OLED| SCL| GPIO 22
SH1106 OLED| VCC| 3.3V
SH1106 OLED| GND| GND

Important: GPS-এর TX → ESP32 RX (GPIO16) এবং GPS-এর RX → ESP32 TX (GPIO17) হবে—TX-to-TX নয়।

আর "DATABASE_URL"-এ তোমার দেওয়া Firebase URL ঠিকভাবে বসানো হয়েছে। "API_KEY"-এর জায়গায় তোমার key বসিয়ে compile/upload করার আগে Firebase Realtime Database Rules এবং Authentication configuration-ও মিলিয়ে নিও; শুধু API key থাকলেই সব database setup-এ write permission নিশ্চিত হয় না।
