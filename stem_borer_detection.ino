/*
  IoT-Based Stem Borer Detection and Monitoring System
  Board: ESP32 DevKit
  Inputs: KY-037 analog output, DHT22
  Output: relay module for light trap
  Telegram: UniversalTelegramBot

  IMPORTANT:
  - Frequency limits and amplitude thresholds are calibration placeholders.
  - A KY-037 is not a validated stem-borer identification sensor.
  - Keep AUTO_TRAP_ENABLED false until field validation is complete.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <DHT.h>

// ---------- USER CONFIGURATION ----------
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
#define BOT_TOKEN "PASTE_TELEGRAM_BOT_TOKEN_HERE"
const String CHAT_ID = "YOUR_NUMERIC_CHAT_ID";

// ---------- PIN CONFIGURATION ----------
#define MIC_PIN 34       // KY-037 AO; ADC1 pin
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define RELAY_PIN 26

// Change this if your relay is active HIGH.
const bool RELAY_ACTIVE_LOW = true;

// ---------- DETECTION CONFIGURATION ----------
// These are starting values for bench calibration, not biological reference values.
const float MIN_DETECTION_HZ = 80.0;
const float MAX_DETECTION_HZ = 1200.0;
const int MIN_PEAK_TO_PEAK = 120;       // 12-bit ADC counts; calibrate experimentally
const int REQUIRED_DETECTIONS = 3;
const unsigned long SAMPLE_WINDOW_MS = 250;
const unsigned long DETECTION_HOLD_MS = 5000;
const unsigned long SENSOR_INTERVAL_MS = 2500;
const unsigned long TELEGRAM_POLL_MS = 1200;
const unsigned long TRAP_ON_TIME_MS = 30000;

// Keep false until acoustic detection is validated.
const bool AUTO_TRAP_ENABLED = false;

// ---------- GLOBALS ----------
DHT dht(DHT_PIN, DHT_TYPE);
WiFiClientSecure securedClient;
UniversalTelegramBot bot(BOT_TOKEN, securedClient);

float temperatureC = NAN;
float humidityPct = NAN;
float estimatedFrequencyHz = 0.0;
int micPeakToPeak = 0;
bool acousticActivity = false;
bool trapIsOn = false;

int consecutiveDetections = 0;
unsigned long lastDetectionMs = 0;
unsigned long trapStartedMs = 0;
unsigned long lastSensorMs = 0;
unsigned long lastTelegramPollMs = 0;

void setTrap(bool on) {
  trapIsOn = on;
  int activeLevel = RELAY_ACTIVE_LOW ? LOW : HIGH;
  int inactiveLevel = RELAY_ACTIVE_LOW ? HIGH : LOW;
  digitalWrite(RELAY_PIN, on ? activeLevel : inactiveLevel);
  if (on) trapStartedMs = millis();
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Wi-Fi connected. IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection timed out. Will retry in loop.");
  }
}

void sampleMicrophone() {
  // Estimate periodicity from rising crossings around the measured midpoint.
  // This is deliberately simple and should be replaced by validated DSP for deployment.
  unsigned long startMs = millis();
  int minValue = 4095;
  int maxValue = 0;
  unsigned long crossingTimes[100];
  int crossingCount = 0;

  // First pass: collect samples and amplitude range.
  const int sampleCount = 1200;
  int samples[ sampleCount ];
  for (int i = 0; i < sampleCount; i++) {
    int value = analogRead(MIC_PIN);
    samples[i] = value;
    if (value < minValue) minValue = value;
    if (value > maxValue) maxValue = value;
    delayMicroseconds(200); // nominal ~5 ksample/s; actual rate depends on execution overhead
  }

  micPeakToPeak = maxValue - minValue;
  int midpoint = (maxValue + minValue) / 2;
  bool wasBelow = samples[0] < midpoint;

  // Detect upward midpoint crossings.
  for (int i = 1; i < sampleCount && crossingCount < 100; i++) {
    bool isAbove = samples[i] >= midpoint;
    if (wasBelow && isAbove) {
      crossingTimes[crossingCount++] = micros();
    }
    wasBelow = !isAbove;
  }

  // Estimate frequency from average crossing interval.
  if (crossingCount >= 3 && micPeakToPeak >= MIN_PEAK_TO_PEAK) {
    // The samples were taken at approximately 5 kHz. Use sample-index intervals
    // rather than micros() timestamps to reduce timestamp-call overhead.
    int crossingIndices[100];
    int count = 0;
    wasBelow = samples[0] < midpoint;
    for (int i = 1; i < sampleCount && count < 100; i++) {
      bool isAbove = samples[i] >= midpoint;
      if (wasBelow && isAbove) crossingIndices[count++] = i;
      wasBelow = !isAbove;
    }

    unsigned long intervalSum = 0;
    int intervals = 0;
    for (int i = 1; i < count; i++) {
      int delta = crossingIndices[i] - crossingIndices[i - 1];
      if (delta > 0) {
        intervalSum += delta;
        intervals++;
      }
    }

    if (intervals > 0) {
      float averageSamples = (float)intervalSum / intervals;
      estimatedFrequencyHz = 5000.0 / averageSamples;
    } else {
      estimatedFrequencyHz = 0.0;
    }
  } else {
    estimatedFrequencyHz = 0.0;
  }

  bool inConfiguredBand =
    estimatedFrequencyHz >= MIN_DETECTION_HZ &&
    estimatedFrequencyHz <= MAX_DETECTION_HZ &&
    micPeakToPeak >= MIN_PEAK_TO_PEAK;

  if (inConfiguredBand) {
    consecutiveDetections++;
    lastDetectionMs = millis();
  } else {
    consecutiveDetections = 0;
  }

  acousticActivity = (consecutiveDetections >= REQUIRED_DETECTIONS) ||
                     (millis() - lastDetectionMs <= DETECTION_HOLD_MS);

  Serial.print("Mic p-p: ");
  Serial.print(micPeakToPeak);
  Serial.print(" | Estimated frequency: ");
  Serial.print(estimatedFrequencyHz, 1);
  Serial.print(" Hz | Activity: ");
  Serial.println(acousticActivity ? "YES" : "NO");

  (void)startMs;
}

void readEnvironment() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h)) humidityPct = h;
  if (!isnan(t)) temperatureC = t;
}

String statusMessage(bool detailed) {
  String message = "STEM BORER MONITOR\n";
  if (isnan(temperatureC)) message += "Temperature: unavailable\n";
  else message += "Temperature: " + String(temperatureC, 1) + " C\n";

  if (isnan(humidityPct)) message += "Humidity: unavailable\n";
  else message += "Humidity: " + String(humidityPct, 1) + " %\n";

  message += "Estimated acoustic frequency: " + String(estimatedFrequencyHz, 1) + " Hz\n";
  message += "Mic peak-to-peak: " + String(micPeakToPeak) + "\n";
  message += "Acoustic activity flag: " + String(acousticActivity ? "YES" : "NO") + "\n";
  message += "Light trap relay: " + String(trapIsOn ? "ON" : "OFF");

  if (detailed) {
    message += "\n\nNote: experimental acoustic activity indicator only; not confirmed pest identification.";
    message += "\nAuto trap: ";
    message += AUTO_TRAP_ENABLED ? "ENABLED" : "DISABLED";
  }
  return message;
}

void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String chatId = bot.messages[i].chat_id;
    String text = bot.messages[i].text;
    text.trim();
    text.toLowerCase();

    if (chatId != CHAT_ID) {
      bot.sendMessage(chatId, "Unauthorized chat. Ask the project owner to configure your access.", "");
      continue;
    }

    if (text == "hi" || text == "hello" || text == "/start") {
      bot.sendMessage(chatId, "Hello! STEM Borer monitoring node is online.\nSend /status for live readings.", "");
    } else if (text == "/status" || text == "status") {
      bot.sendMessage(chatId, statusMessage(true), "");
    } else if (text == "/trap_on") {
      setTrap(true);
      bot.sendMessage(chatId, "Light-trap relay switched ON.", "");
    } else if (text == "/trap_off") {
      setTrap(false);
      bot.sendMessage(chatId, "Light-trap relay switched OFF.", "");
    } else {
      bot.sendMessage(chatId, "Commands: hi, /status, /trap_on, /trap_off", "");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  analogReadResolution(12);
  pinMode(MIC_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  setTrap(false);

  dht.begin();
  connectWiFi();

  // Telegram uses HTTPS. setInsecure() skips certificate validation and is
  // convenient for a student prototype, but is not recommended for production.
  securedClient.setInsecure();

  Serial.println("STEM Borer monitoring prototype started.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  unsigned long now = millis();

  if (now - lastSensorMs >= SENSOR_INTERVAL_MS) {
    lastSensorMs = now;
    readEnvironment();
    sampleMicrophone();

    if (AUTO_TRAP_ENABLED && acousticActivity && !trapIsOn) {
      setTrap(true);
    }
  }

  if (trapIsOn && millis() - trapStartedMs >= TRAP_ON_TIME_MS) {
    setTrap(false);
  }

  if (WiFi.status() == WL_CONNECTED && now - lastTelegramPollMs >= TELEGRAM_POLL_MS) {
    lastTelegramPollMs = now;
    int messages = bot.getUpdates(bot.last_message_received + 1);
    while (messages) {
      handleNewMessages(messages);
      messages = bot.getUpdates(bot.last_message_received + 1);
    }
  }

  delay(10);
}
