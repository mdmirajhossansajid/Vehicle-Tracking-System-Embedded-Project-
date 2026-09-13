#include <WiFi.h>
#include <FirebaseESP32.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h> 

// --- EXTRACTED CREDENTIAL CONFIG ---
#define WIFI_SSID "technolab"
#define WIFI_PASSWORD "12345678"

#define DATABASE_URL "https://vehicle-tracking-system-baac1-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyA1-eCoOIz0a2cxDe3LsMV3aG_e-7Ioyug"

// --- PIN CONFIGURATION ---
#define RXD2 16
#define TXD2 17
#define i2c_Address 0x3c 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// --- OBJECTS ---
TinyGPSPlus gps;
HardwareSerial SerialGPS(2); 
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;
FirebaseJson json;

unsigned long lastUpdateTime = 0;

void setup() {

    Serial.begin(115200);
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Initialize 1.3" OLED Display unit
  display.begin(i2c_Address, true); 
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

}
void loop() {

}
void updateSystem() {
    
}
