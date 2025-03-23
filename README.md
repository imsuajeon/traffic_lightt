### 수아의 우당탕탕 신호등!
---
##### 이번주 수아의 유튭:


---
##### 아두이노 추가 코드

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
    
시리얼 명령어로 모드 변경을 처리하는 코드를 추가했으며 MODE:로 시작하는 문자열이 있으면 모드 변경을 실시한다. 모드벼 처리하는 조건문을 추가했다. 사실 그전 코드 그대로 갖고와서 시리얼 명령어만 바꿨다. (핸드포즈에서 시리얼 보내면 여기를 거쳐서 모드가 결정된다.)

---
##### html 추가코드
    <script src="https://unpkg.com/@gohai/p5.webserial@^1/libraries/p5.webserial.js"></script>
    <script src="https://unpkg.com/ml5@0.6.0/dist/ml5.min.js"></script>

p5.js에서 웹 브라우저와 Arduino 간 시리얼 통신을 가능하게 해주는 라이브러리와 웹에서 머신러닝을 쉽게 사용할 수 있도록 만든 고수준 라이브러리 추가

---
##### P5 추가 코드
      let handPose;
      let video;
      let hands = [];
      video = createCapture(VIDEO, { flipped: true });
      handPose = ml5.handpose(video, modelReady);
      handPose.on("predict", gotHands);

웹캠으로 손 인식하고 Handpose 모델로 손가락 keypoint 받아오는용

image(video, camX, camY, camW, camH);
      fill(255, 0, 0);
      noStroke();
        
화면에 뜨는 카메라

  for (let i = 0; i < hands.length; i++) {
    for (let [xVal, yVal] of hands[i].landmarks) {
      let flippedX = camX + (video.width - xVal);
      let drawY = camY + yVal;
      circle(flippedX, drawY, 10);
    }
  }
  
손 인식 결과

if (hands.length > 0 && millis() > gestureCooldown) {
    let newMode = detectModeGesture(hands[0]);
    if (newMode && newMode !== currentMode) {
      gestureCooldown = millis() + 2000;
      currentMode = newMode;
      sendModeCommandMode(newMode);
      console.log("Mode changed via gesture:", newMode);
          }
        }
        
모드 변경을 위한 제스처
  
  if (hands.length > 0 && millis() > sliderGestureCooldown) {
    for (let hand of hands) {
    if (isThumbandIndexExtended(hand)) {                                              // 엄지 검지 위로 피면
        let val = max(redSlider.value() - 100, parseInt(redSlider.elt.min));            // 빨간색 시간을 줄임
        redSlider.value(val); sendRedTime(); 
        console.log("엄지검지 위");  
      }else if (isPalmDown(hand)) {                                                    // 손바닥을 아래로 내리면 
        let val = max(greenSlider.value() - 100, parseInt(greenSlider.elt.min));        // 초록색 시간을 줄임
        greenSlider.value(val); sendGreenTime();                                    
        console.log("손바닥 아래");                               
      } else if (isThumbExtended(hand)) {                                               // 엄지만 올리면
        let val = min(yellowSlider.value() + 100, parseInt(yellowSlider.elt.max));      // 노란색 시간을 늘림
        yellowSlider.value(val); sendYellowTime();                              
        console.log("엄지 위");
      } else if (isThumbDown(hand)) {                                                   // 엄지만 내리면                 
        let val = max(yellowSlider.value() - 100, parseInt(yellowSlider.elt.min));      // 노란색 시간을 줄임
        yellowSlider.value(val); sendYellowTime();
        console.log("엄지 아래");
      } else if (isIndexandPinkyExtended(hand)) {                                       // 검지와 새끼만 올리면
        let val = min(redSlider.value() + 100, parseInt(redSlider.elt.max));            // 빨간색 시간을 늘림
        redSlider.value(val); sendRedTime();
        console.log("검지 새끼 위");
      } else if (isPalmUp(hand)) {                                                       // 손바닥을 위로 올리면
        let val = min(greenSlider.value() + 100, parseInt(greenSlider.elt.max));        // 초록색 시간을 늘림
        greenSlider.value(val); sendGreenTime();                                        // 시리얼로 전송      
        console.log("손바닥 위");                                                        // 콘솔에 로그 출력
      } 
      
슬라이드 제어
제스쳐로 슬라이더 바꿀때 100씩 바뀜
      
function detectModeGesture(hand) {                                                      
  const lm = hand.landmarks;                             // 손의 랜드마크 정보
  const isUp = (tip, dip) => lm[tip][1] < lm[dip][1];    // 손가락이 펴져있는지 확인하는 함수
  const idx = isUp(8, 6), mid = isUp(12, 10), rng = isUp(16, 14), pink = isUp(20, 18);  // 각 손가락의 상태를 저장
  if (!idx && !mid && !rng && !pink) return "NORMAL";   // 모든 손가락이 접혀있으면 NORMAL
  if (idx && !mid && !rng && !pink) return "EMERGENCY"; // 검지만 펴져있으면 EMERGENCY
  if (idx && mid && !rng && !pink) return "BLINKING";   // 검지와 중지만 펴져있으면 BLINKING
  if (idx && mid && rng && !pink) return "ON_OFF";      // 검지, 중지, 약지가 펴져있으면 ON_OFF
  return null;
}

모드 제스처 인식 함수

      function isIndexandPinkyExtended(hand) {    // 검지와 새끼 위로로 핀 함수      
  let lm = hand.landmarks;
  return (lm[8][1] < lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] < lm[18][1]);
}
      function isThumbandIndexExtended(hand) {   // 검지와 새끼 아래로 핀 함수
  let lm = hand.landmarks;
  return (lm[16][1] > lm[14][1] && lm[12][1] > lm[10][1] && lm[8][1] < lm[6][1] && lm[20][1] > lm[18][1] && lm[4][1] < lm[3][1]);
}
      function isThumbExtended(hand) {           // 엄지 위로 핀 함수
  let lm = hand.landmarks;
  return (lm[4][1] < lm[3][1] && lm[8][1] > lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] < lm[18][1]);
}
      function isThumbDown(hand) {               // 엄지 아래로 핀 함수
  let lm = hand.landmarks;
  return lm[4][1] > lm[3][1];
}
      function isPalmUp(hand) {                  // 손바닥 위로
  let lm = hand.landmarks;
  return (lm[4][1] < lm[3][1] && lm[8][1] < lm[6][1] && lm[12][1] < lm[10][1] && lm[16][1] < lm[14][1] && lm[20][1] < lm[18][1]);
}
      function isPalmDown(hand) {                // 손바닥 아래로
  let lm = hand.landmarks;
  return (lm[4][1] > lm[3][1] && lm[8][1] > lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] > lm[18][1]);
}

슬라이더 제스쳐 인식 함수

---
##### 제스쳐 설명

![image](https://github.com/user-attachments/assets/6052f0a7-fffe-4c73-a3a5-c9e54a49f5a2)
신호등모드
![image](https://github.com/user-attachments/assets/2066722f-f9ba-44cc-b0de-ab7ef4818223)
Emergency 모드
![image](https://github.com/user-attachments/assets/b229b472-6ade-4e6f-99e0-e3e64b84b33a)
blink 모드
![image](https://github.com/user-attachments/assets/0f05ebf9-d709-493d-a539-d87d168cc07c)
On/Off
![image](https://github.com/user-attachments/assets/bcbfe236-c07f-4dd4-9c12-5bef495b6ed5)
빨간색 주기 늘리기
![image](https://github.com/user-attachments/assets/1fe68d2f-0dbf-410e-b71f-e8aa42756b2c)
빨간색 주기 줄이기
![image](https://github.com/user-attachments/assets/fd643ebc-d9fb-4ad1-bec4-53c72e4224cc)
노란색 주기 늘리기
![image](https://github.com/user-attachments/assets/9d9c7782-63a3-4dd2-9c54-67ca8106d2eb)
노란색 주기 줄이기
![image](https://github.com/user-attachments/assets/847cead4-aea4-4341-a5a2-0312daec9453)
초록색 주기 늘리기
![image](https://github.com/user-attachments/assets/e9769e9e-9313-4250-b357-4c93b263c1ce)
초록색 주기 줄이기

---


### 수아의 얼렁뚱땅 신호등!
---
##### 수아의 유튭:
https://www.youtube.com/watch?si=_yvx8b5839RY8pO_&v=_kSP3t_Ftbo&feature=youtu.be

---

##### 신호등 설명: 
Arduino를 이용하여 신호등(교통신호) 시스템을 구현하고, p5.js를 통해 LED 상태와 모드를 시각적으로 모니터링 및 제어할 수 있는 시스템이다.

빨강(R), 노랑(Y), 초록(G) LED와 가변저항, 버튼 3개, 아두이노, 컴퓨터를 이용해 신호등과 유사한 기능을 구현하며 버튼은 긴급 정지, 깜빡임 모드, 전원 On/Off 등 다양한 동작을 토글 방식으로 수행한다.

동작 사이클은 빨강-노랑-초록-노랑 순으로 진행되며, 각 LED의 점등 시간과 깜빡임 패턴을 시각적으로 확인할 수 있다.

웹 브라우저와 시리얼 통신을 통해 입력값(가변저항, 버튼)에 따른 LED 상태가 실시간으로 반영된다.

---
##### 하드웨어 구성 요소

- Arduino Uno (또는 호환 보드)
- LED 3개
  - 빨간 LED: Arduino 디지털 핀 9 사용
  - 노란 LED: Arduino 디지털 핀 10 사용
  - 초록 LED: Arduino 디지털 핀 11 사용
- 푸시 버튼 3개
  - 버튼 1: Arduino 디지털 핀 2 (비상모드 토글)
  - 버튼 2: Arduino 디지털 핀 3 (깜빡임 모드 토글)
  - 버튼 3: Arduino 디지털 핀 4 (신호등 On/Off 토글)
- 가변저항 (Potentiometer)
  - 아날로그 입력 A4에 연결 (LED 밝기 제어)
- 저항
  - 각 LED에 220Ω 저항(LED 보호용)
- 브레드보드와 점퍼 와이어

---

##### 회로 구성 및 연결 방법
![image](https://github.com/user-attachments/assets/d587a619-57c8-4062-9fa8-8399c46c114f)

각 버튼은 내부 풀업 저항을 사용하도록 설정되어 있다. (코드에서 `INPUT_PULLUP` 사용)  
즉, 버튼이 눌리지 않은 기본 상태는 HIGH이며, 버튼을 누르면 LOW 신호로 읽힌다.

---

##### 신호 레벨 및 풀업/풀다운 설정

- 버튼 신호 (INPUT_PULLUP)
  - Default Signal: 버튼이 연결된 핀은 기본적으로 HIGH 상태이다.
  - 동작 원리: 버튼이 눌리면 해당 핀이 GND에 연결되어 LOW 신호가 감지된다.
  
- LED 출력
  - LED는 Arduino의 디지털 출력 핀(9, 10, 11)에서 PWM 신호를 통해 밝기를 제어한다.
  - `analogWrite()` 함수를 이용하여 가변저항에서 읽은 값(0~255)을 LED 밝기로 설정한다.

---

##### 소프트웨어 구성 개요

###### Arduino 코드

- 라이브러리: 
  - `TaskScheduler`: LED 타이밍 및 모드 전환을 위한 스케줄링 관리.
  - `PinChangeInterrupt`: 버튼 입력의 핀 체인지 인터럽트를 사용하여 빠른 반응 구현.
  
- 모드:
  - 신호등 모드: 자동으로 빨강 → 노랑 → 초록 → 초록 깜빡이 → 노랑 순환.
  - 긴급 모드: 버튼 1로 수동 제어(빨간 LED 강제 ON).
  - 깜빡임 모드: 버튼 2로 모든 LED 깜빡임 구현 후 종료 시 "realOFFF" 메시지로 신호등 모드로 복귀.
  - On/Off 모드: 버튼 3로 전체 LED ON/OFF 전환.

- 시리얼 통신:
  - p5.js와의 시리얼 통신을 통해 LED 타이밍 값 및 밝기 정보를 주고받는다.

###### p5.js 코드

- GUI 구성: 
  - 슬라이더를 통해 각 LED의 지속 시간을 조정할 수 있으며, 변경 값은 Arduino로 전송된다.
  - 캔버스에 LED 상태(빨간, 노란, 초록)와 현재 모드, 밝기 값이 표시된다.
  
- 시리얼 통신:
  - Web Serial API를 이용해 Arduino와 연결되며, 수신된 데이터를 실시간으로 파싱하여 화면에 업데이트한다.

---

##### 구축 및 실행 방법


1. Arduino 코드 업로드
   - Arduino IDE를 열고, 제공된 Arduino 코드를 붙여넣은 후 보드에 업로드한다.
   
2. p5.js 인터페이스 실행:
   - p5.js 코드를 p5.js 웹 에디터 또는 로컬 환경에서 실행한다.
   - "Connect to Arduino" 버튼을 눌러 시리얼 포트를 연결하고, 슬라이더로 LED 타이밍을 조절한다.

3. 테스트 및 검증:
   - 각 버튼(버튼 1, 2, 3)을 눌러 모드 전환이 올바르게 이루어지는지 확인한다.
   - Arduino에서 전송되는 상태 메시지에 따라 p5.js 화면에 LED 상태와 모드가 올바르게 표시되는지 점검한다.

---

##### 짜잔!
![image](https://github.com/user-attachments/assets/f760d4b2-e780-4c1b-86e7-eefae111836b)
![image](https://github.com/user-attachments/assets/449f2ab9-f98e-4378-bf01-5dc195b33671)
![image](https://github.com/user-attachments/assets/4d744c83-ad26-4fc0-b0c1-f5dae4975c01)
![image](https://github.com/user-attachments/assets/440e9393-fd68-4903-9011-ea6d910c89ef)



