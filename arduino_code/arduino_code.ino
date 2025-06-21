#include <Servo.h>

const int buttonPin = 13;      // QR 버튼
const int servoPin  = 9;       // MG90S 신호 핀

Servo doorServo;

// 상태 머신
enum State { IDLE, OPENING, WAITING, CLOSING };
State state = IDLE;
unsigned long stateStartTime = 0;

// 서보 중립·속도 펄스 (μs) — 반드시 실제 장비로 보정하세요
const int SERVO_STOP  = 1500;  // Neutral (정지)
const int SERVO_OPEN  = 1650;  // “열림” 방향 속도
const int SERVO_CLOSE = 1350;  // “닫힘” 방향 속도

// 타이밍 상수
// 1) 한 바퀴(360°) 도는 시간 (ms) — 직접 측정 후 여기에 넣으세요
const unsigned long FULL_ROTATION_TIME = 2000UL;
// 2) 90° 회전 시간 = 360° 시간 / 4
// 예시 캘리브레이션 값 (ms 단위)
const unsigned long OPEN_TIME  = 270;  // 실제 측정: 90° 여는 데 270ms 걸림
const unsigned long CLOSE_TIME = 430;  // 실제 측정: 90° 닫는 데 430ms 걸림

// 3) 문 열림 후 Hold 시간 (ms)
const unsigned long OPEN_HOLD_TIME      = 1000UL;                 // 1초

// 디바운스
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

// 시리얼 입력 버퍼
String inputBuffer;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);

  doorServo.attach(servoPin);
  doorServo.writeMicroseconds(SERVO_STOP);  // 초기 정지
}

void loop() {
  unsigned long now = millis();

  // ── 1) SERIAL “UNLOCK” 처리 ──
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      inputBuffer.trim();
      if (inputBuffer == "UNLOCK") startSequence(now);
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  // ── 2) BUTTON SHOW_QR 처리 (논블로킹) ──
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }
  if (now - lastDebounceTime > debounceDelay) {
    static bool pressed = false;
    if (reading == LOW && !pressed) {
      pressed = true;
      Serial.println("SHOW_QR");
    }
    if (reading == HIGH) pressed = false;
  }
  lastButtonState = reading;

  // ── 3) 상태 머신 ──
  switch (state) {
    case OPENING:
      // 90° 열림 회전 완료
      if (now - stateStartTime >= OPEN_TIME) {
        doorServo.writeMicroseconds(SERVO_STOP);  // 정지
        state = WAITING;
        stateStartTime = now;
      }
      break;

    case WAITING:
      // 열린 상태로 Hold
      if (now - stateStartTime >= OPEN_HOLD_TIME) {
        doorServo.writeMicroseconds(SERVO_CLOSE); // 반대 방향 회전 시작
        state = CLOSING;
        stateStartTime = now;
      }
      break;

    case CLOSING:
      // 90° 닫힘 회전 완료
      if (now - stateStartTime >= CLOSE_TIME) {
        doorServo.writeMicroseconds(SERVO_STOP);  // 정지
        state = IDLE;
      }
      break;

    case IDLE:
    default:
      // 대기 상태
      break;
  }
}

// 시리얼 UNLOCK 또는 다른 트리거로 호출
void startSequence(unsigned long timestamp) {
  if (state != IDLE) return;                // 이미 동작 중이면 무시
  doorServo.writeMicroseconds(SERVO_OPEN);   // 열림 회전 시작
  state = OPENING;
  stateStartTime = timestamp;
}
