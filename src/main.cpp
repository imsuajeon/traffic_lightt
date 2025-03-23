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
const int GREEN_BLINK_INTERVAL = 333; // (예상 blink 시간)
const int GREEN_BLINK_COUNT = 3;

// TaskScheduler 및 밝기 변수
Scheduler ts;
int brightness = 0; 

// LED 상태 변수
bool redState = false;
bool yellowState = false;
bool greenState = false;

// 현재 모드 문자열 (Serial 전송 및 확인용)
String currentMode = "신호등";

// LED 상태를 전송 
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

  // 각 버튼에 대한 핀 체인지 인터럽트 (Falling Edge)
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
    String line = Serial.readStringUntil('\n');
    line.trim();
    
    if (line.startsWith("RED ")) {                    // RED 시간 변경  
      String numStr = line.substring(4);
      int val = numStr.toInt();
      if (val > 0) {
        RED_TIME = val;
        red.setInterval(RED_TIME);
        Serial.print("Updated RED_TIME=");
        Serial.println(RED_TIME);
      }
    }
    else if (line.startsWith("YELLOW ")) {            // YELLOW 시간 변경
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
    else if (line.startsWith("GREEN ")) {             // GREEN 시간 변경
      String numStr = line.substring(6);
      int val = numStr.toInt();
      if (val > 0) {
        GREEN_TIME = val;
        green.setInterval(GREEN_TIME);
        Serial.print("Updated GREEN_TIME=");
        Serial.println(GREEN_TIME);
      }
    }
    else if (line.startsWith("MODE:")) {              // 모드 변경
      String modeCmd = line.substring(5);
      modeCmd.trim();
      currentMode = modeCmd;
      Serial.print("Mode received: ");
      Serial.println(modeCmd);
      
      // p5.js에 모드 업데이트를 위한 메시지 전송
      Serial.print("MODE ");
      Serial.println(currentMode);
      
      if (modeCmd == "NORMAL") {
        trafficLightOn = true;
        toggleMode = false;
        blinkMode = false;
        red.restartDelayed();
        Serial.println("Switched to NORMAL mode.");
      }
      else if (modeCmd == "EMERGENCY") {
        trafficLightOn = false;
        toggleMode = true;
        blinkMode = false;
        red.disable();
        yellow.disable();
        green.disable();
        greenBlink.disable();
        yellowAfterBlink.disable();
        analogWrite(redPin, brightness);
        analogWrite(yellowPin, 0);
        analogWrite(greenPin, 0);
        redState = true;
        yellowState = false;
        greenState = false;
        sendState();
        Serial.print("MODE ");
        Serial.println(currentMode);  
        Serial.println("Manual Mode: RED_ON, YELLOW_OFF, GREEN_OFF");
      }
      else if (modeCmd == "BLINKING") {
        trafficLightOn = false;
        blinkMode = true;
        red.disable();
        yellow.disable();
        green.disable();
        greenBlink.disable();
        yellowAfterBlink.disable();
        Serial.println("Switched to BLINKING mode.");
        // blinking 모드 처리는 비차단 방식으로 개선하는 것이 좋습니다.
        while (blinkMode) {
          analogWrite(redPin, brightness);
          analogWrite(yellowPin, brightness);
          analogWrite(greenPin, brightness);
          redState = true;
          yellowState = true;
          greenState = true;
          sendState();
          Serial.println("ALL_ON");
          delay(ALL_BLINK_INTERVAL);
          if (Serial.available() > 0) {
            blinkMode = false;
            break;
          }
          analogWrite(redPin, 0);
          analogWrite(yellowPin, 0);
          analogWrite(greenPin, 0);
          redState = false;
          yellowState = false;
          greenState = false;
          sendState();
          Serial.println("ALL_OFF");
          delay(ALL_BLINK_INTERVAL);
          if (Serial.available() > 0) {
            blinkMode = false;
            break;
          }
        }
        Serial.println("Exiting BLINKING mode.");
      }
      else if (modeCmd == "ON_OFF") {
        trafficLightOn = false;
        red.disable();
        yellow.disable();
        green.disable();
        greenBlink.disable();
        yellowAfterBlink.disable();
        analogWrite(redPin, 0);
        analogWrite(yellowPin, 0);
        analogWrite(greenPin, 0);
        redState = false;
        yellowState = false;
        greenState = false;
        sendState();
        Serial.print("MODE ");
        Serial.println(currentMode);  // 버튼처럼 모드 메시지 재전송
        Serial.println("Switched to OFF mode.");
      }
    }
    
    
    else { // 숫자 메시지: 깜빡임 간격 변경
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

  // 버튼 처리
  if (buttonPressed1) {
    buttonPressed1 = false;
    toggleMode = !toggleMode;
    if (toggleMode) {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable();
      analogWrite(redPin, brightness); 
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0); 
      redState = true; yellowState = false; greenState = false;
      sendState();
      Serial.println("Manual Mode: RED_ON, YELLOW_OFF, GREEN_OFF");
      Serial.print("MODE ");
      Serial.println("EMERGENCY");
    } else {
      analogWrite(redPin, 0);
      redState = false;
      sendState();
      Serial.println("Manual Mode Off: RED_OFF");
      Serial.print("MODE ");
      Serial.println("NORMAL");
      red.restartDelayed();
    }
  }

  if (buttonPressed2) {
    buttonPressed2 = false;
    blinkMode = !blinkMode;
    if (blinkMode) {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable();
      Serial.print("MODE ");
      Serial.println("BLINKING");
      while (blinkMode) {
        // 모두 켜기
        analogWrite(redPin, brightness);
        analogWrite(yellowPin, brightness);
        analogWrite(greenPin, brightness);
        redState = true; yellowState = true; greenState = true;
        sendState();
        Serial.println("ALL_ON");
        delay(ALL_BLINK_INTERVAL);
        // 모두 끄기
        analogWrite(redPin, 0);
        analogWrite(yellowPin, 0);
        analogWrite(greenPin, 0);
        redState = false; yellowState = false; greenState = false;
        sendState();
        Serial.println("ALL_OFF");
        delay(ALL_BLINK_INTERVAL);
        if (buttonPressed2) {
          buttonPressed2 = false;
          blinkMode = false;
          break;
        }
        if (Serial.available() > 0) {  // 새로운 제스처 감지 시 blinking 종료
          blinkMode = false;
          break;
        }
      }
    } else {
      analogWrite(redPin, 0);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0);
      redState = false; yellowState = false; greenState = false;
      sendState();
      Serial.println("realOFFF");
      Serial.print("MODE ");
      Serial.println("NORMAL");
      red.restartDelayed();
    }
  }

  if (buttonPressed3) {
    buttonPressed3 = false;
    trafficLightOn = !trafficLightOn;
    if (trafficLightOn) {
      sendState();
      Serial.println("ONNNN");
      Serial.print("MODE ");
      Serial.println("NORMAL");
      red.restartDelayed();
    } else {
      red.disable(); yellow.disable(); green.disable();
      greenBlink.disable(); yellowAfterBlink.disable();
      analogWrite(redPin, 0);
      analogWrite(yellowPin, 0);
      analogWrite(greenPin, 0);
      Serial.print("MODE ");
      Serial.println("ON_OFF");
      redState = false; yellowState = false; greenState = false;
      sendState();
      Serial.println("OFFWhite");
    }
  }

  // TaskScheduler 실행 (신호등 모드일 때만)
  if (!toggleMode && !blinkMode && trafficLightOn) {
    ts.execute();
  }
}

// Task 콜백 함수들
bool redOE() {
  analogWrite(redPin, brightness);
  redState = true;
  sendState();
  Serial.println("RED_ON");
  return true;
}
void redOD() {
  analogWrite(redPin, 0);
  redState = false;
  sendState();
  Serial.println("RED_OFF");
  yellow.restartDelayed();
}
bool yellowOE() {
  analogWrite(yellowPin, brightness);
  yellowState = true;
  sendState();
  Serial.println("YELLOW_ON");
  return true;
}
void yellowOD() {
  analogWrite(yellowPin, 0);
  yellowState = false;
  sendState();
  Serial.println("YELLOW_OFF");
  green.restartDelayed();
}
bool greenOE() {
  analogWrite(greenPin, brightness);
  greenState = true;
  sendState();
  Serial.println("GREEN_ON");
  return true;
}
void greenOD() {
  analogWrite(greenPin, 0);
  greenState = false;
  sendState();
  Serial.println("GREEN_OFF");
  greenBlink.restartDelayed();
}
bool greenBlinkOE() {
  analogWrite(greenPin, 0);
  return true;
}
void greenBlinkCB() {
  greenState = !greenState;
  analogWrite(greenPin, greenState ? brightness : 0);
  sendState();
  if (greenState) {
    Serial.println("GREEN_ON (Blink)");
  } else {
    Serial.println("GREEN_OFF (Blink)");
  }
}
void greenBlinkOD() {
  analogWrite(greenPin, 0);
  greenState = false;
  sendState();
  Serial.println("GREEN_OFF (Blink End)");
  yellowAfterBlink.restartDelayed();
}
bool yellowOE2() {
  analogWrite(yellowPin, brightness);
  yellowState = true;
  sendState();
  Serial.println("YELLOW_ON");
  return true;
}
void yellowOD2() {
  analogWrite(yellowPin, 0);
  yellowState = false;
  sendState();
  Serial.println("YELLOW_OFF");
  red.restartDelayed();
}
