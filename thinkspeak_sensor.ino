#include <ESP8266WiFi.h>
#include <DHT.h>
#include "ThingSpeak.h"

// ---------------- Wi-Fi Credentials ----------------
#define WIFI_SSID "Kaapi"
#define WIFI_PASS "coys@2202428"

// ---------------- ThingSpeak Setup ----------------
unsigned long channelID = 3141076;   
const char *writeAPIKey = "U3XMIHYN56OW2OYD";  

WiFiClient client;

// ---------------- Sensor Pins ----------------
#define DHTPIN D4        // DHT22 data pin connected to D4
#define DHTTYPE DHT22
#define FLAME_PIN D5     // Flame sensor digital output
#define MQ135_PIN A0     // MQ135 analog output

DHT dht(DHTPIN, DHTTYPE);


void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected... reconnecting");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {
      delay(1000);
      Serial.print(".");
      retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi reconnected!");
    } else {
      Serial.println("\nFailed to reconnect to WiFi");
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(FLAME_PIN, INPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.println(WiFi.localIP());
  ThingSpeak.begin(client);

  Serial.println("System Initialized");
}

void loop() {
  ensureWiFiConnected(); 

  // --- Sensor Readings ---
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int gasValue = analogRead(MQ135_PIN);
  int flameDetected = digitalRead(FLAME_PIN);

 
  Serial.println("======================================");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Air Quality (MQ135): ");
  Serial.println(gasValue);

  if (gasValue < 200) {
    Serial.println("Air Quality: Excellent ");
  } else if (gasValue < 400) {
    Serial.println("Air Quality: Good ");
  } else if (gasValue < 700) {
    Serial.println("Air Quality: Moderate ");
  } else if (gasValue < 900) {
    Serial.println("Air Quality: Poor ");
  } else {
    Serial.println("Air Quality: Hazardous ");
  }

  Serial.print("Flame Detected: ");
  if (flameDetected == HIGH) {
    Serial.println("YES - Flame Detected!");
  } else {
    Serial.println("No Flame");
  }

  // --- Send data to ThingSpeak ---
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, gasValue);
  ThingSpeak.setField(4, flameDetected);

  int response = ThingSpeak.writeFields(channelID, writeAPIKey);

  if (response == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.print("Problem sending data. HTTP error code: ");
    Serial.println(response);
  }

  // ThingSpeak requires a minimum 15s gap — 20s is safe
  delay(20000);
}
