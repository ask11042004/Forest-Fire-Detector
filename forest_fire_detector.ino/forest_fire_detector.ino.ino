#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "DHT.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi credentials
#define WIFI_SSID "realme 9i 5G"
#define WIFI_PASSWORD "0987654321"

// Firebase project credentials
#define API_KEY "AIzaSyDwDffyW59V595XypiKeKe8REcQWfB3cA0"
#define DATABASE_URL "https://emergency-networking-default-rtdb.asia-southeast1.firebasedatabase.app"

// Firebase user authentication
#define USER_EMAIL "a********@gmail.com"
#define USER_PASSWORD "A####"

// Sensor pins
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ2_PIN 34
#define FLAME_DO_PIN 5
#define PIR_PIN 13

// Fire detection thresholds
#define TEMP_THRESHOLD 30.5
#define HUMIDITY_THRESHOLD 80.0
#define SMOKE_THRESHOLD 1500
#define FLAME_DETECTED 0

DHT dht(DHTPIN, DHTTYPE);

// Initialize LCD (address 0x27, 16 columns, 2 rows)
// If 0x27 doesn't work, try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(FLAME_DO_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Forest Fire");
  lcd.setCursor(0, 1);
  lcd.print("System Starting");
  delay(2000);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWi-Fi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(1000);

  // Setup Firebase authentication
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Setup complete");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1000);
}

void loop() {
  // Read sensor values
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int smoke = analogRead(MQ2_PIN);
  int flame = digitalRead(FLAME_DO_PIN);
  int motion = digitalRead(PIR_PIN);

  if (isnan(temperature)) temperature = 0.0;
  if (isnan(humidity)) humidity = 0.0;

  // Print sensor values to Serial
  Serial.printf("Temperature: %.2f°C\n", temperature);
  Serial.printf("Humidity: %.2f%%\n", humidity);
  Serial.printf("Smoke Level: %d\n", smoke);
  Serial.printf("Flame Sensor: %d\n", flame);
  Serial.printf("PIR Motion: %d\n", motion);

  // Fire detection logic
  bool fireDetected = (temperature > TEMP_THRESHOLD) && 
                      (humidity < HUMIDITY_THRESHOLD);

  // Display on LCD
  lcd.clear();
  
  if (fireDetected) {
    // Show FIRE ALERT
    lcd.setCursor(0, 0);
    lcd.print("*** FIRE !!! ***");
    lcd.setCursor(0, 1);
    lcd.print("ALERT DETECTED!");
    
    Serial.println("🚨 ALERT: FOREST FIRE DETECTED! 🚨");
  } else {
    // Show sensor values
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C H:");
    lcd.print((int)humidity);
    lcd.print("%");
    
    lcd.setCursor(0, 1);
    lcd.print("S:");
    lcd.print(smoke);
    lcd.print(" F:");
    lcd.print(flame);
    lcd.print(" M:");
    lcd.print(motion);
  }

  // Upload to Firebase
  if (Firebase.ready()) {
    Firebase.RTDB.setFloat(&fbdo, "/sensorData/temperature", temperature);
    Firebase.RTDB.setFloat(&fbdo, "/sensorData/humidity", humidity);
    Firebase.RTDB.setInt(&fbdo, "/sensorData/smokeLevel", smoke);
    Firebase.RTDB.setInt(&fbdo, "/sensorData/flameDigital", flame);
    Firebase.RTDB.setInt(&fbdo, "/sensorData/motionDetected", motion);

    if (fireDetected) {
      Firebase.RTDB.setString(&fbdo, "/alert", "🔥 FOREST FIRE DETECTED! 🔥");
    } else {
      Firebase.RTDB.setString(&fbdo, "/alert", "No fire detected - All clear");
    }
  } else {
    Serial.println("Firebase not ready");
  }

  Serial.println("---");
  delay(2000);
}