#include <WiFi.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include "DHT.h"
#include <LiquidCrystal_I2C.h>

const char* ssid = "W";
const char* password = "11111111";

String phoneNumber = "+628xxxxxx"; //Phone Number
String apiKey = "9101314";

#define DHTPIN 15    
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
#define RAINDROP_PIN 34

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long previousMillis = 0;
const long interval = 1000;

unsigned long gerimisStartTime = 0;
unsigned long hujanStartTime = 0;
unsigned long hujanLebatStartTime = 0;

int gerimisCount = 0;
int hujanCount = 0;
int hujanLebatCount = 0;

void sendMessage(String message) {
  String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);    
  HTTPClient http;
  http.begin(url);

  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpResponseCode = http.POST("");
  if (httpResponseCode == 200){
    Serial.println("Pesan berhasil dikirim");
  } else {
    Serial.println("Kesalahan mengirim pesan");
    Serial.print("Kode respons HTTP: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Menghubungkan");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Terhubung ke jaringan WiFi dengan Alamat IP: ");
  Serial.println(WiFi.localIP());

  dht.begin();
  
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Kelompok 4");
  lcd.setCursor(0, 1);
  lcd.print("Mini Stasiun");
  delay(2000);
  lcd.clear();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t)) {
      Serial.println(F("Gagal membaca dari sensor DHT!"));
      lcd.setCursor(0, 0);
      lcd.print("Kesalahan DHT");
      delay(2000);
      lcd.clear();
      return;
    }

    int raindropValue = analogRead(RAINDROP_PIN);
    int intensitasHujan = raindropValue - 2000;

    Serial.print("Kelembaban: ");
    Serial.println(h);
    Serial.print("Suhu: ");
    Serial.println(t);
    Serial.print("Intensitas Hujan: ");
    Serial.println(intensitasHujan);

    Serial.print(h);
    Serial.print(",");
    Serial.print(t);
    Serial.print(",");
    Serial.println(intensitasHujan);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Lembab: ");
    lcd.print(h);
    lcd.print(" %");
    lcd.setCursor(0, 1);
    lcd.print("Suhu: ");
    lcd.print(t);
    lcd.print(" C");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Intensitas Hujan:");
    lcd.print(intensitasHujan);
    lcd.setCursor(0, 1);
    if (intensitasHujan <= 300) {
      lcd.print("Hujan Lebat");
    } else if (intensitasHujan <= 800) {
      lcd.print("Hujan");
    } else if (intensitasHujan <= 1500) {
      lcd.print("Gerimis");
    } else {
      lcd.print("Tidak Hujan");
    }
    delay(1000);
    lcd.clear();

    if (intensitasHujan <= 1500) {
      if (gerimisStartTime == 0) gerimisStartTime = currentMillis;
      if (currentMillis - gerimisStartTime >= 10000 && gerimisCount < 5) {
        String message = "GERIMIS! Peringatan Cuaca! Kelembaban: " + String(h) + "%, Suhu: " + String(t) + "C, Intensitas Hujan: Gerimis!";
        sendMessage(message);
        gerimisCount++;
        gerimisStartTime = currentMillis;
      }
    } else {
      gerimisStartTime = 0;
      gerimisCount = 0;
    }

    if (intensitasHujan <= 800) {
      if (hujanStartTime == 0) hujanStartTime = currentMillis;
      if (currentMillis - hujanStartTime >= 50000 && hujanCount < 10) {
        String message = "HUJAN!! Peringatan Cuaca! Kelembaban: " + String(h) + "%, Suhu: " + String(t) + "C, Intensitas Hujan: Hujan!" + String(intensitasHujan);
        sendMessage(message);
        hujanCount++;
        hujanStartTime = currentMillis;
      }
    } else {
      hujanStartTime = 0;
      hujanCount = 0;
    }

    if (intensitasHujan <= 300) {
      if (hujanLebatStartTime == 0) hujanLebatStartTime = currentMillis;
      if (currentMillis - hujanLebatStartTime >= 100000 && hujanLebatCount < 10) {
        String message = "HUJAN LEBAT!!! Peringatan Cuaca! Kelembaban: " + String(h) + "%, Suhu: " + String(t) + "C, Intensitas Hujan: Hujan Lebat!" + String(intensitasHujan);
        sendMessage(message);
        hujanLebatCount++;
        hujanLebatStartTime = currentMillis;
      }
    } else {
      hujanLebatStartTime = 0;
      hujanLebatCount = 0;
    }
  }
}
