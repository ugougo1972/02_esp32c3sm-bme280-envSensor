const int PIN_ENC_A  = 32;
const int PIN_ENC_B  = 33;
const int PIN_ENC_SW = 25;

int lastA = HIGH;
int lastSW = HIGH;
long encoderCount = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);

  lastA = digitalRead(PIN_ENC_A);
  lastSW = digitalRead(PIN_ENC_SW);

  Serial.println("Rotary encoder test start");
}

void loop() {
  int currentA = digitalRead(PIN_ENC_A);

  if (currentA != lastA) {
    int currentB = digitalRead(PIN_ENC_B);

    if (currentB != currentA) {
      encoderCount++;
      Serial.print("CW  Count = ");
      Serial.println(encoderCount);
    } else {
      encoderCount--;
      Serial.print("CCW Count = ");
      Serial.println(encoderCount);
    }
  }
  lastA = currentA;

  int currentSW = digitalRead(PIN_ENC_SW);

  if (lastSW == HIGH && currentSW == LOW) {
    Serial.println("BUTTON: PUSH");
  }
  if (lastSW == LOW && currentSW == HIGH) {
    Serial.println("BUTTON: RELEASE");
  }
  lastSW = currentSW;

  delay(1);
}