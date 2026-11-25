#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

String message = "Halo, ini notifikasi dari SIPILAH ESP32!";

int trig1 = 12, echo1 = 14;
int trig2 = 27, echo2 = 26;
int trig3 = 25, echo3 = 33;

// Status kapasitas tong (dalam cm)
const int batas_penuh = 10;
bool sudahKirim1 = false;
bool sudahKirim2 = false;
bool sudahKirim3 = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");

  pinMode(trig1, OUTPUT); pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT); pinMode(echo2, INPUT);
  pinMode(trig3, OUTPUT); pinMode(echo3, INPUT);
}

void loop() {
  int jarak1 = bacaJarak(trig1, echo1);
  int jarak2 = bacaJarak(trig2, echo2);
  int jarak3 = bacaJarak(trig3, echo3);

  Serial.printf("Kompartemen 1: %d cm | Kompartemen 2: %d cm | Kompartemen 3: %d cm\n", jarak1, jarak2, jarak3);

  cekTong(jarak1, "Logam / B3", sudahKirim1);
  cekTong(jarak2, "Anorganik", sudahKirim2);
  cekTong(jarak3, "Organik", sudahKirim3);
}

int bacaJarak(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long durasi = pulseIn(echoPin, HIGH, 30000);
  if (durasi == 0) return -1;
  return durasi * 0.034 / 2;
}

void cekTong(int jarak, String nama, bool &sudahKirim) {
  if (jarak > 0 && jarak < batas_penuh && !sudahKirim) {
    kirimTelegram("⚠️ Tong " + nama + " sudah penuh!");
    sudahKirim = true;
  } else if (jarak > 20 && sudahKirim) {
    sudahKirim = false;
    Serial.println("✅ Tong " + nama + " sudah dikosongkan.");
  }
}

void kirimTelegram(String pesan) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = "https://api.telegram.org/bot" + BOT_TOKEN + "/sendMessage?chat_id=" + CHAT_ID + "&text=" + urlEncode(pesan);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) Serial.println("📤 Notifikasi terkirim!");
    else Serial.println("❌ Gagal kirim.");
    http.end();
  }
}

String urlEncode(String text) {
  String encoded = "";
  char code[] = "0123456789ABCDEF";
  for (int i = 0; i < text.length(); i++) {
    char c = text.charAt(i);
    if (isalnum(c)) encoded += c;
    else {
      encoded += '%';
      encoded += code[(c >> 4) & 0xF];
      encoded += code[c & 0xF];
    }
  }
  return encoded;
}