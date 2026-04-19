const int BUTTON_ROAD1 = 2;
const int BUTTON_ROAD2 = 3;
const int BUTTON_ROAD3 = 4;
const int BUTTON_ROAD4 = 5;

unsigned long lastPress = 0;
const unsigned long debounceTime = 1000;

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_ROAD1, INPUT_PULLUP);
  pinMode(BUTTON_ROAD2, INPUT_PULLUP);
  pinMode(BUTTON_ROAD3, INPUT_PULLUP);
  pinMode(BUTTON_ROAD4, INPUT_PULLUP);
  
  delay(1000);
  Serial.println("Ambulance transmitter ready");
}

void loop() {
  unsigned long now = millis();
  
  if (now - lastPress > debounceTime) {
    if (digitalRead(BUTTON_ROAD1) == LOW) {
      Serial.println("A");
      lastPress = now;
      delay(100);
    }
    else if (digitalRead(BUTTON_ROAD2) == LOW) {
      Serial.println("B");
      lastPress = now;
      delay(100);
    }
    else if (digitalRead(BUTTON_ROAD3) == LOW) {
      Serial.println("C");
      lastPress = now;
      delay(100);
    }
    else if (digitalRead(BUTTON_ROAD4) == LOW) {
      Serial.println("D");
      lastPress = now;
      delay(100);
    }
  }
  
  delay(50);
}
