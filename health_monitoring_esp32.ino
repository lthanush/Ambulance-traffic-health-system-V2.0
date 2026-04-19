#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "MAX30105.h"
#include "heartRate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
const char* serverHost = "192.168.1.100";
const int serverPort = 80;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;

double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int SpO2 = 0;
double ESpO2 = 95.0;
double FSpO2 = 0.7;
double frate = 0.95;
int i = 0;

unsigned long lastDisplayUpdate = 0;
unsigned long lastServerUpdate = 0;
const unsigned long DISPLAY_INTERVAL = 1000;
const unsigned long SERVER_INTERVAL = 5000;

WiFiClient client;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Health Monitor");
  display.println("Initializing...");
  display.display();
  delay(2000);
  
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor Error!");
    display.println("Check wiring");
    display.display();
    while (1);
  }
  
  byte ledBrightness = 0x1F;
  byte sampleAverage = 4;
  byte ledMode = 2;
  int sampleRate = 100;
  int pulseWidth = 411;
  int adcRange = 4096;
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);
  
  Serial.println("Sensor initialized");
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting WiFi");
  display.display();
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Connected");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);
  } else {
    Serial.println("\nWiFi Failed");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Failed");
    display.println("Offline Mode");
    display.display();
    delay(2000);
  }
  
  Serial.println("System Ready");
}

void loop() {
  unsigned long currentTime = millis();
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();
  
  if (irValue > 50000) {
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      
      beatsPerMinute = 60 / (delta / 1000.0);
      
      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;
        
        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
    
    double fred = (double)redValue;
    double fir = (double)irValue;
    avered = avered * frate + fred * (1.0 - frate);
    aveir = aveir * frate + fir * (1.0 - frate);
    
    sumredrms += (fred - avered) * (fred - avered);
    sumirrms += (fir - aveir) * (fir - aveir);
    
    i++;
    
    if ((i % 100) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      SpO2 = (int)((-23.3 * R + 110) * 0.9);
      
      if (SpO2 > 100) SpO2 = 100;
      if (SpO2 < 0) SpO2 = 0;
      
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;
      
      sumredrms = 0.0;
      sumirrms = 0.0;
      i = 0;
    }
    
    if (currentTime - lastDisplayUpdate > DISPLAY_INTERVAL) {
      updateDisplay(beatAvg, (int)ESpO2, true);
      lastDisplayUpdate = currentTime;
    }
    
    if (currentTime - lastServerUpdate > SERVER_INTERVAL) {
      if (WiFi.status() == WL_CONNECTED) {
        sendDataToHospital(beatAvg, (int)ESpO2);
      }
      lastServerUpdate = currentTime;
    }
    
  } else {
    if (currentTime - lastDisplayUpdate > DISPLAY_INTERVAL) {
      updateDisplay(0, 0, false);
      lastDisplayUpdate = currentTime;
    }
  }
}

void updateDisplay(int heartRate, int spo2, bool fingerDetected) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Health Monitor");
  display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
  
  if (fingerDetected) {
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.print("Heart Rate:");
    display.setTextSize(2);
    display.setCursor(0, 28);
    display.print(heartRate);
    display.setTextSize(1);
    display.print(" BPM");
    
    display.setTextSize(1);
    display.setCursor(0, 48);
    display.print("SpO2:");
    display.setTextSize(2);
    display.setCursor(50, 48);
    display.print(spo2);
    display.setTextSize(1);
    display.print(" %");
    
    if (WiFi.status() == WL_CONNECTED) {
      display.setCursor(110, 0);
      display.print("W");
    }
  } else {
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.println("Place");
    display.setCursor(10, 45);
    display.println("Finger");
  }
  
  display.display();
}

void sendDataToHospital(int heartRate, int spo2) {
  Serial.println("Sending data to hospital...");
  
  if (client.connect(serverHost, serverPort)) {
    String jsonData = "{\"heartRate\":" + String(heartRate) + 
                      ",\"spo2\":" + String(spo2) + 
                      ",\"timestamp\":" + String(millis()) + "}";
    
    client.println("POST /patient-data HTTP/1.1");
    client.print("Host: ");
    client.println(serverHost);
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonData.length());
    client.println("Connection: close");
    client.println();
    client.println(jsonData);
    
    Serial.println("Data sent successfully");
    Serial.print("Heart Rate: ");
    Serial.print(heartRate);
    Serial.print(" BPM, SpO2: ");
    Serial.print(spo2);
    Serial.println("%");
    
    client.stop();
  } else {
    Serial.println("Connection to hospital failed");
  }
}
