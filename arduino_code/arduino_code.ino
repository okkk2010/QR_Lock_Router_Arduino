// 개선된 예시
const int buttonPin = 13;
const int motorPinA  = 9;   // BLDC 모터 A 채널 (PWM)
const int motorPinB  = 10;  // BLDC 모터 B 채널 (PWM)
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
const unsigned long unlockDuration = 1000;


String inputBuffer;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(motorPinA, OUTPUT);
  pinMode(motorPinB, OUTPUT);
  stopMotor();
}


void loop() {
  // 1) Serial → buffer
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputBuffer.trim();
      if (inputBuffer == "UNLOCK") unlockDoor();
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  // ② 버튼 논블로킹 처리
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }
  if (millis() - lastDebounceTime > debounceDelay) {
    static bool buttonPressed = false;
    if (reading == LOW && !buttonPressed) {
      buttonPressed = true;
      Serial.println("SHOW_QR");
    }
    if (reading == HIGH) {
      buttonPressed = false;
    }
  }
  lastButtonState = reading;
}

void unlockDoor() {
  // PWM 세기(0~255) 조정 가능
  analogWrite(motorPinA, 100);
  analogWrite(motorPinB, 0);
  delay(unlockDuration);
  stopMotor();
}

// 모터 정지
void stopMotor() {
  analogWrite(motorPinA, 0);
  analogWrite(motorPinB, 0);
}
