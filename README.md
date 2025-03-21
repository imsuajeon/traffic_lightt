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



