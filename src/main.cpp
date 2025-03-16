#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// 핀/가변저항/버튼 설정
const int redPin = 9;
const int yellowPin = 10;
const int greenPin = 11;
const int button1 = 2;
const int button2 = 3;
const int button3 = 4;
const int potPin = A4;

// LED 시간 설정
int RED_TIME = 2000;    
int YELLOW_TIME = 500;  
int GREEN_TIME = 2000;  
int ALL_BLINK_INTERVAL = 500;  // 깜빡임 간격 (2번 버튼)
const int GREEN_BLINK_INTERVAL = 333; // 제가 이해한 시간이 맞나요?
const int GREEN_BLINK_COUNT = 3;

// TaskScheduler 및 밝기 변수
Scheduler ts;
int brightness = 0; 

// LED 상태 변수
bool redState = false;
bool yellowState = false;
bool greenState = false;

// LED 상태를 전송 (밝기는 별도 메시지로 보냄)
void sendState() {
  Serial.print("STATE ");
  Serial.print(redState ? "1" : "0");
  Serial.print(",");
  Serial.print(yellowState ? "1" : "0");
  Serial.print(",");
  Serial.println(greenState ? "1" : "0");
}

// Task 콜백 선언부
bool redOE();       void redOD();
bool yellowOE();    void yellowOD();
bool yellowOE2();   void yellowOD2();
bool greenOE();     void greenOD();
bool greenBlinkOE();
void greenBlinkCB();
void greenBlinkOD();

// Task 객체 생성부
Task red(RED_TIME, TASK_ONCE, NULL, &ts, false, redOE, redOD);
Task yellow(YELLOW_TIME, TASK_ONCE, NULL, &ts, false, yellowOE, yellowOD);
Task green(GREEN_TIME, TASK_ONCE, NULL, &ts, false, greenOE, greenOD);
Task greenBlink(GREEN_BLINK_INTERVAL, GREEN_BLINK_COUNT * 2, greenBlinkCB, &ts, false, greenBlinkOE, greenBlinkOD);
Task yellowAfterBlink(YELLOW_TIME, TASK_ONCE, NULL, &ts, false, yellowOE2, yellowOD2);

// 버튼 / 모드 관련 변수
volatile bool buttonPressed1 = false;
volatile bool buttonPressed2 = false;
volatile bool buttonPressed3 = false;
volatile bool toggleMode = false;
volatile bool blinkMode = false;
volatile bool trafficLightOn = true;

// 버튼 인터럽트 핸들러
void handleButtonPress1() { buttonPressed1 = true; }
void handleButtonPress2() { buttonPressed2 = true; }
void handleButtonPress3() { buttonPressed3 = true; }

void setup() {
  Serial.begin(9600); // 시리얼 통신

  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  // 각 버튼에 대한 핀 체인지 인터럽트랄까 (Falling Edge)
  attachPCINT(digitalPinToPCINT(button1), handleButtonPress1, FALLING);
  attachPCINT(digitalPinToPCINT(button2), handleButtonPress2, FALLING);
  attachPCINT(digitalPinToPCINT(button3), handleButtonPress3, FALLING);

  // 각 LED 지속 시간
  red.setInterval(RED_TIME);
  yellow.setInterval(YELLOW_TIME);
  green.setInterval(GREEN_TIME);

  // 빨간불부터 시작!
  red.restartDelayed();
}

void loop() {
  // p5.js에서 들어오는 시리얼 데이터 처리
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n'); // 한 줄 읽기
    line.trim();
    if (line.startsWith("RED ")) { // RED 문구가 있으면
      String numStr = line.substring(4); // 숫자만 꺼내서
      int val = numStr.toInt(); // 정수로
      if (val > 0) {
        RED_TIME = val; // 그걸 이제 RED_TIME으로
        red.setInterval(RED_TIME);
        Serial.print("Updated RED_TIME=");
        Serial.println(RED_TIME);
      }
    }
    else if (line.startsWith("YELLOW ")) { // 이하 동일
      String numStr = line.substring(7);
      int val = numStr.toInt();
      if (val > 0) {
        YELLOW_TIME = val;
        yellow.setInterval(YELLOW_TIME);
        yellowAfterBlink.setInterval(YELLOW_TIME);
        Serial.print("Updated YELLOW_TIME=");
        Serial.println(YELLOW_TIME);
      }
    }
    else if (line.startsWith("GREEN ")) { // 이하 동일
      String numStr = line.substring(6);
      int val = numStr.toInt();
      if (val > 0) {
        GREEN_TIME = val;
        green.setInterval(GREEN_TIME);
        Serial.print("Updated GREEN_TIME=");
        Serial.println(GREEN_TIME);
      }
    }
    else { // 메시지가 없다면 깜빡임으로
      int incomingInterval = line.toInt();
      if (incomingInterval > 0) {
        ALL_BLINK_INTERVAL = incomingInterval;
        Serial.print("Blink interval set to: ");
        Serial.println(ALL_BLINK_INTERVAL);
      }
    }
  }

  // 가변저항 읽어서 밝기 설정
  int potValue = analogRead(potPin);
  delay(100);
  brightness = map(potValue, 0, 1023, 0, 255);
  
  // 현재 밝기 전송
  Serial.print("BRIGHTNESS ");
  Serial.println(brightness);

  // 대망의 버튼
  if (buttonPressed1) { // 1번 버튼 눌리면
    buttonPressed1 = false;
    toggleMode = !toggleMode;
    if (toggleMode) {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable(); // 모든 task 죽여버리고
      // 빨간불만 켜줌
      analogWrite(redPin, brightness); 
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0); 
      redState = true; yellowState = false; greenState = false;
      sendState();
      Serial.println("Manual Mode: RED_ON, YELLOW_OFF, GREEN_OFF");
    } else { // 한 번 더 눌리면
      analogWrite(redPin, 0);
      redState = false; // 빨간불 꺼짐
      sendState();
      Serial.println("Manual Mode Off: RED_OFF");
      red.restartDelayed(); // 그리고 다시 신호등 시작작
    }
  }

  if (buttonPressed2) { // 2번 버튼 눌리면
    buttonPressed2 = false;
    blinkMode = !blinkMode;
    if (blinkMode) {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable(); // 모든 task 죽여버리고

      while (blinkMode) {
        // 모두 켜는거랑랑
        analogWrite(redPin, brightness);
        analogWrite(yellowPin, brightness);
        analogWrite(greenPin, brightness);
        redState = true; yellowState = true; greenState = true;
        sendState();
        Serial.println("ALL_ON");
        delay(ALL_BLINK_INTERVAL);

        // 모두 끄는거 반복
        analogWrite(redPin, 0);
        analogWrite(yellowPin, 0);
        analogWrite(greenPin, 0);
        redState = false; yellowState = false; greenState = false;
        sendState();
        Serial.println("ALL_OFF");
        delay(ALL_BLINK_INTERVAL);

        if (buttonPressed2) { // 또 눌리면 
          buttonPressed2 = false;
          blinkMode = false; // 블링크 탈출출
          break;
        }
      }
    } else { // 틸츨히면 
      analogWrite(redPin, 0);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0); // led 다 꺼버리고
      redState = false; yellowState = false; greenState = false;
      sendState();
      Serial.println("realOFFF");
      red.restartDelayed(); // 다시 신호등 시작
    }
  }

  if (buttonPressed3) { // 3번 버튼이 눌리면면
    buttonPressed3 = false;
    trafficLightOn = !trafficLightOn;
    if (trafficLightOn) {
      sendState();
      Serial.println("ONNNN");
      red.restartDelayed(); // 신호등 시작!
    } else {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable(); // 모든 task 죽여버리고

      analogWrite(redPin, 0);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0);
      redState = false; yellowState = false; greenState = false;
      sendState();
      Serial.println("OFFWhite"); // 걍 다 끔끔
    }
  }

  // TaskScheduler 실행 
  if (!toggleMode && !blinkMode && trafficLightOn) {
    ts.execute(); // 실행
  }
}

// Task 콜백 함수들 (task 시작과 종료 지정정)
bool redOE() { // 빨간불 켜기기
  analogWrite(redPin, brightness);
  redState = true;
  sendState();
  Serial.println("RED_ON");
  return true;
}
void redOD() { // 빨간불 끄기 + 노란불 시작
  analogWrite(redPin, 0);
  redState = false;
  sendState();
  Serial.println("RED_OFF");
  yellow.restartDelayed();
}
bool yellowOE() { // 노란불 켜기
  analogWrite(yellowPin, brightness);
  yellowState = true;
  sendState();
  Serial.println("YELLOW_ON");
  return true;
}
void yellowOD() { // 노란불 끄기 + 초록불 시작
  analogWrite(yellowPin, 0);
  yellowState = false;
  sendState();
  Serial.println("YELLOW_OFF");
  green.restartDelayed();
}
bool greenOE() { // 초록불 켜기
  analogWrite(greenPin, brightness);
  greenState = true;
  sendState();
  Serial.println("GREEN_ON");
  return true;
}
void greenOD() { // 초록불 끄기 + 깜빡임 시작
  analogWrite(greenPin, 0);
  greenState = false;
  sendState();
  Serial.println("GREEN_OFF");
  greenBlink.restartDelayed();
}
bool greenBlinkOE() { // 초록불 깜빡임 시작
  analogWrite(greenPin, 0);
  return true;
}
void greenBlinkCB() { // 초록불 깜빡깜빡
  greenState = !greenState;
  analogWrite(greenPin, greenState ? brightness : 0);
  sendState();
  if (greenState) {
    Serial.println("GREEN_ON (Blink)");
  } else {
    Serial.println("GREEN_OFF (Blink)");
  }
}
void greenBlinkOD() { // 초록불 깜빡임 끝 + 노란불 시작
  analogWrite(greenPin, 0);
  greenState = false;
  sendState();
  Serial.println("GREEN_OFF (Blink End)");
  yellowAfterBlink.restartDelayed();
}
bool yellowOE2() { // 노란불 켜기
  analogWrite(yellowPin, brightness);
  yellowState = true;
  sendState();
  Serial.println("YELLOW_ON");
  return true;
}
void yellowOD2() { // 노란불 끄기 + 빨간불 시작
  analogWrite(yellowPin, 0);
  yellowState = false;
  sendState();
  Serial.println("YELLOW_OFF");
  red.restartDelayed();
}
