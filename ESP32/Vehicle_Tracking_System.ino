#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// =====================================================
// Wi-Fi
// =====================================================
#define WIFI_SSID "name"
#define WIFI_PASSWORD "12345678"

// =====================================================
// Firebase
// =====================================================
#define DATABASE_URL ""
#define API_KEY ""

// =====================================================
// GPS
// =====================================================
#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600

// =====================================================
// OLED
// =====================================================
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_ADDRESS 0x3C

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// =====================================================
// Objects
// =====================================================
TinyGPSPlus gps;
HardwareSerial SerialGPS(2);

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
FirebaseJson json;

// =====================================================
// Timing
// =====================================================
unsigned long lastUpdateTime = 0;
const unsigned long UPDATE_INTERVAL = 2000;

// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);

  // -------------------------
  // GPS
  // -------------------------
  SerialGPS.begin(
    GPS_BAUD,
    SERIAL_8N1,
    RXD2,
    TXD2
  );

  // -------------------------
  // OLED
  // -------------------------
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(I2C_ADDRESS, true)) {
    Serial.println("OLED initialization failed!");
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");
  display.println("System Starting...");

  display.display();

  delay(1500);

  // -------------------------
  // Wi-Fi connecting screen
  // -------------------------
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

  // -------------------------
  // Wi-Fi result
  // -------------------------
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

  // -------------------------
  // Firebase configuration
  // -------------------------
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;

  Serial.println("Connecting to Firebase...");

  // Anonymous Firebase authentication
  if (Firebase.signUp(
        &config,
        &auth,
        "",
        ""
      )) {

    Serial.println(
      "Firebase anonymous authentication successful."
    );

  } else {

    Serial.print(
      "Firebase authentication failed: "
    );

    Serial.println(
      config.signer.signupError.message.c_str()
    );
  }

  Firebase.begin(&config, &auth);

  Firebase.reconnectNetwork(true);

  Serial.println("Firebase initialized.");

  // -------------------------
  // Wait for Firebase
  // -------------------------
  unsigned long firebaseStart = millis();

  while (
    !Firebase.ready() &&
    millis() - firebaseStart < 15000
  ) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (Firebase.ready()) {

    Serial.println("Firebase READY!");

  } else {

    Serial.println("Firebase is not ready yet.");
  }

  // -------------------------
  // System ready
  // -------------------------
  display.clearDisplay();
  display.setCursor(0, 5);

  display.println("VEHICLE TRACKING");
  display.println("--------------------");
  display.println("System Ready!");
  display.println("GPS Searching...");

  display.display();

  Serial.println("System Ready.");
}

// =====================================================
// Loop
// =====================================================
void loop() {

  // -------------------------
  // Read GPS continuously
  // -------------------------
  while (SerialGPS.available() > 0) {

    char gpsData = SerialGPS.read();

    gps.encode(gpsData);
  }

  updateSystem();
}

// =====================================================
// Main system update
// =====================================================
void updateSystem() {

  int satellitesCount = gps.satellites.value();

  bool locationValid = gps.location.isValid();

  // -------------------------
  // GPS not fixed
  // -------------------------
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

    if ((millis() / 1000) % 2 == 0) {
      display.print(" .");
    }

    display.display();

    return;
  }

  // -------------------------
  // GPS valid
  // -------------------------
  float latitude = gps.location.lat();
  float longitude = gps.location.lng();

  float rawSpeed = gps.speed.kmph();

  float filteredSpeed = rawSpeed;

  if (rawSpeed < 3.0) {
    filteredSpeed = 0.0;
  }

  // -------------------------
  // OLED live display
  // -------------------------
  display.clearDisplay();

  display.setTextSize(2);

  display.setCursor(15, 0);

  display.print(filteredSpeed, 1);

  display.setTextSize(1);

  display.print(" km/h");

  display.drawLine(
    0,
    20,
    128,
    20,
    SH110X_WHITE
  );

  display.setCursor(0, 28);

  display.print("Lat: ");
  display.println(latitude, 6);

  display.print("Lng: ");
  display.println(longitude, 6);

  display.print("Sats: ");
  display.println(satellitesCount);

  display.display();

  // -------------------------
  // Firebase update interval
  // -------------------------
  if (
    millis() - lastUpdateTime < UPDATE_INTERVAL
  ) {
    return;
  }

  lastUpdateTime = millis();

  // -------------------------
  // Firebase ready check
  // -------------------------
  if (!Firebase.ready()) {

    Serial.println(
      "Firebase is not ready yet; update skipped."
    );

    return;
  }

  // -------------------------
  // Create JSON
  // -------------------------
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

  json.set(
    "timestamp/.sv",
    "timestamp"
  );

  // -------------------------
  // Serial information
  // -------------------------
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

  // -------------------------
  // History upload
  // Only when vehicle is moving
  // -------------------------
  if (filteredSpeed > 0.0) {

    if (
      Firebase.RTDB.pushJSON(
        &firebaseData,
        "/history",
        &json
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

  // -------------------------
  // Live GPS upload
  // -------------------------
  if (
    Firebase.RTDB.setJSON(
      &firebaseData,
      "/gps",
      &json
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

  // -------------------------
  // Heartbeat
  // -------------------------
  if (
    Firebase.RTDB.setInt(
      &firebaseData,
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
