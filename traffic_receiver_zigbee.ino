#include <LiquidCrystal.h>

LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

const int RED1 = 2,  YEL1 = 3,  GRN1 = 4;
const int RED2 = 5,  YEL2 = 6,  GRN2 = 7;
const int RED3 = 8,  YEL3 = 9,  GRN3 = 10;
const int RED4 = 11, YEL4 = 12, GRN4 = 13;

const unsigned long NORMAL_GREEN  = 10000UL;
const unsigned long NORMAL_YELLOW =  2000UL;
const unsigned long ALL_RED_GAP   =  2000UL;
const unsigned long EMERG_GREEN   = 15000UL;
const unsigned long EMERG_YELLOW  =  3000UL;

bool ambulanceFlag = false;
int  ambulanceRoad = 0;
int  currentRoad   = 1;

char lastZigBeeCmd = ' ';
unsigned long lastCmdTime = 0;
const unsigned long DEBOUNCE_MS = 500UL;

void setSignal(int road, int state) {
  int r, y, g;
  switch (road) {
    case 1: r = RED1; y = YEL1; g = GRN1; break;
    case 2: r = RED2; y = YEL2; g = GRN2; break;
    case 3: r = RED3; y = YEL3; g = GRN3; break;
    case 4: r = RED4; y = YEL4; g = GRN4; break;
    default: return;
  }
  digitalWrite(r, state == 0 ? HIGH : LOW);
  digitalWrite(y, state == 1 ? HIGH : LOW);
  digitalWrite(g, state == 2 ? HIGH : LOW);
}

void allRed() {
  for (int i = 1; i <= 4; i++) setSignal(i, 0);
}

void lcdPrint(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  char buf[17];
  strncpy(buf, line1, 16);
  buf[16] = '\0';
  lcd.print(buf);
  lcd.setCursor(0, 1);
  strncpy(buf, line2, 16);
  buf[16] = '\0';
  lcd.print(buf);
}

void printTimestamp() {
  unsigned long sec = millis() / 1000;
  Serial.print("[");
  Serial.print(sec);
  Serial.print("s] ");
}

void checkZigBee() {
  if (Serial1.available() > 0) {
    char ch = Serial1.read();
    unsigned long now = millis();

    if (ch == lastZigBeeCmd && (now - lastCmdTime) < DEBOUNCE_MS) return;

    lastZigBeeCmd = ch;
    lastCmdTime = now;

    if (ch >= 'A' && ch <= 'D') {
      ambulanceRoad = ch - 'A' + 1;
      ambulanceFlag = true;
      printTimestamp();
      Serial.print("RFID detected: Ambulance on Road ");
      Serial.println(ambulanceRoad);
    }
  }
}

void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkZigBee();
    delay(100);
  }
}

void runNormalPhase(int road) {
  char line1[17], line2[17];

  allRed();
  setSignal(road, 1);
  snprintf(line1, 17, "Road %d: READY", road);
  snprintf(line2, 17, "Others: RED");
  lcdPrint(line1, line2);
  printTimestamp();
  Serial.print("Road "); Serial.print(road); Serial.println(" -> YELLOW");
  safeDelay(NORMAL_YELLOW);

  if (ambulanceFlag) {
    printTimestamp();
    Serial.println("RFID detected during yellow");
    return;
  }

  setSignal(road, 2);
  snprintf(line1, 17, "Road %d: GO  ", road);
  lcdPrint(line1, "Timer: 10s");
  printTimestamp();
  Serial.print("Road "); Serial.print(road); Serial.println(" -> GREEN");

  for (int t = 10; t > 0; t--) {
    if (ambulanceFlag) {
      printTimestamp();
      Serial.println("RFID detected during green");
      break;
    }
    snprintf(line2, 17, "Timer: %2ds", t);
    lcd.setCursor(0, 1);
    lcd.print(line2);
    safeDelay(1000);
  }

  setSignal(road, 1);
  snprintf(line1, 17, "Road %d: CLEAR", road);
  lcdPrint(line1, "Others: RED");
  printTimestamp();
  Serial.print("Road "); Serial.print(road); Serial.println(" -> YELLOW clearance");
  safeDelay(NORMAL_YELLOW);

  setSignal(road, 0);
  printTimestamp();
  Serial.print("Road "); Serial.print(road); Serial.println(" -> RED");

  if (!ambulanceFlag) {
    printTimestamp();
    Serial.println("All red safety gap");
    lcdPrint("All Roads: RED", "Safety gap...");
    safeDelay(ALL_RED_GAP);
  }
}

void runAmbulancePhase(int road) {
  char line1[17], line2[17];

  printTimestamp();
  Serial.print("AMBULANCE OVERRIDE: Road ");
  Serial.println(road);

  allRed();
  lcdPrint("!! EMERGENCY !!", "Clearing roads..");
  delay(1000);

  setSignal(road, 1);
  snprintf(line2, 17, "Road %d CLEARING", road);
  lcdPrint("!! AMBULANCE !!", line2);
  printTimestamp();
  Serial.print("Ambulance Road "); Serial.print(road); Serial.println(" -> YELLOW");
  delay(EMERG_YELLOW);

  setSignal(road, 2);
  printTimestamp();
  Serial.print("Ambulance Road "); Serial.print(road); Serial.println(" -> GREEN");

  for (int t = 15; t > 0; t--) {
    snprintf(line2, 17, "Road %d GO: %2ds", road, t);
    lcdPrint("!! AMBULANCE !!", line2);
    delay(1000);
  }

  setSignal(road, 1);
  lcdPrint("Road Clear", "Resuming cycle..");
  printTimestamp();
  Serial.println("Ambulance complete");
  delay(EMERG_YELLOW);

  allRed();
  ambulanceFlag = false;
  ambulanceRoad = 0;
  printTimestamp();
  Serial.println("Resuming normal cycle");

  lcdPrint("All Roads: RED", "Safety gap...");
  delay(ALL_RED_GAP);
}

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);

  int pins[] = {RED1,YEL1,GRN1, RED2,YEL2,GRN2, RED3,YEL3,GRN3, RED4,YEL4,GRN4};
  for (int i = 0; i < 12; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], LOW);
  }

  lcd.begin(16, 2);
  lcdPrint("Traffic Control", "Initializing...");

  Serial.println("Traffic control system ready");
  Serial.println("ZigBee receiver active");

  allRed();
  delay(2000);
  printTimestamp();
  Serial.println("Starting normal cycle\n");
}

void loop() {
  for (int road = 1; road <= 4; road++) {
    currentRoad = road;
    checkZigBee();

    if (ambulanceFlag) {
      runAmbulancePhase(ambulanceRoad);
      continue;
    }

    runNormalPhase(road);

    checkZigBee();
    if (ambulanceFlag) {
      runAmbulancePhase(ambulanceRoad);
      continue;
    }
  }
}
