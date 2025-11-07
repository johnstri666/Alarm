#include <WiFi.h>
#include <WebServer.h>
#include "time.h"

// --- WiFi Config ---
const char* ssid = "SINURLINGGA";
const char* password = "andrenico123";

// --- Pin Output ---
const int ledPin = 21;
const int buzzer1Pin = 23;
const int buzzer2Pin = 22;

// --- Time ---
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;


WebServer server(80);


int alarmHour = -1;
int alarmMinute = -1;
bool alarmActive = false;

/// HTML Page///
String htmlPage() {
  String html = R"(
  <!DOCTYPE html>
  <html>
  <head>
    <title>ESP32 Alarm Clock</title>
    <style>
      body {
        background-color: #111;
        color: white;
        font-family: Arial;
        text-align: center;
        padding-top: 50px;
      }
      input {
        padding: 10px;
        font-size: 18px;
        margin: 5px;
      }
      button {
        padding: 10px 20px;
        background-color: #007bff;
        border: none;
        color: white;
        font-size: 18px;
        border-radius: 5px;
        cursor: pointer;
      }
      button:hover {
        background-color: #0056b3;
      }
    </style>
  </head>
  <body>
    <h1>ESP32 Alarm Control</h1>
    <form action="/set" method="get">
      <label>Set Alarm Time (24H):</label><br>
      <input type="number" name="hour" min="0" max="23" placeholder="Hour" required>
      <input type="number" name="minute" min="0" max="59" placeholder="Minute" required><br>
      <button type="submit">Set Alarm</button>
    </form>
    <h3>Current Alarm: )" + 
    (alarmHour >= 0 ? String(alarmHour) + ":" + (alarmMinute < 10 ? "0" : "") + String(alarmMinute) : "Not Set") +
    R"(</h3>
  </body>
  </html>
  )";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", htmlPage());
}

void handleSetAlarm() {
  if (server.hasArg("hour") && server.hasArg("minute")) {
    alarmHour = server.arg("hour").toInt();
    alarmMinute = server.arg("minute").toInt();
    alarmActive = false;
    Serial.printf("Alarm diset ke: %02d:%02d\n", alarmHour, alarmMinute);
    server.send(200, "text/html", "<h2>Alarm diset ke " + String(alarmHour) + ":" + String(alarmMinute) + 
                 "</h2><br><a href='/'>Kembali</a>");
  } else {
    server.send(400, "text/plain", "Parameter tidak lengkap.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzer1Pin, OUTPUT);
  pinMode(buzzer2Pin, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Menyambung ke WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");
  Serial.print("Akses di: http://");
  Serial.println(WiFi.localIP());

  // Sinkronisasi waktu
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  // Routing web server
  server.on("/", handleRoot);
  server.on("/set", handleSetAlarm);
  server.begin();
}

void loop() {
  server.handleClient();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int currentHour = timeinfo.tm_hour;
  int currentMinute = timeinfo.tm_min;

  // Cek waktu alarm
  if (alarmHour == currentHour && alarmMinute == currentMinute && !alarmActive) {
    alarmActive = true;
    runAlarm();
  }

  // LED berkedip tiap detik
  if (!alarmActive) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

void runAlarm() {
  Serial.println("=== ALARM BERBUNYI ===");
  unsigned long startTime = millis();
  unsigned long duration = 5UL * 60UL * 1000UL;  // 5 menit

  while (millis() - startTime < duration) {
    digitalWrite(ledPin, HIGH);
    digitalWrite(buzzer1Pin, HIGH);
    digitalWrite(buzzer2Pin, HIGH);
    delay(200);
    digitalWrite(buzzer1Pin, LOW);
    digitalWrite(buzzer2Pin, LOW);
    delay(200);
  }

  digitalWrite(buzzer1Pin, LOW);
  digitalWrite(buzzer2Pin, LOW);
  digitalWrite(ledPin, LOW);
  alarmActive = false;
  Serial.println("=== ALARM SELESAI ===");
}
