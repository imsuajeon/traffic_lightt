// 전역 변수 선언언
let redOn = false;
let yellowOn = false;
let greenOn = false;
let brightnessVal = 0;  // Arduino에서 받은 밝기 값
let currentMode = "신호등"; // 초기 모드는 "신호등"
let port;
let reader;
let writer;
let connectButton;
let redSlider, yellowSlider, greenSlider;

// blink 모드 활성 여부를 나타내는 플래그
let isBlinking = false;

function setup() {
  createCanvas(400, 300);

  // 연결 버튼 생성
  connectButton = createButton("Connect to Arduino");
  connectButton.position(10, 10);
  connectButton.mousePressed(connectToSerial);

  // 빨간불 슬라이더
  createP("Red Time (ms)").position(10, 40);
  redSlider = createSlider(300, 5000, 2000, 100); // 300~5000ms, 초기값 2000ms
  redSlider.position(10, 70);
  redSlider.input(sendRedTime); 

  // 노란불 슬라이더
  createP("Yellow Time (ms)").position(10, 90);
  yellowSlider = createSlider(300, 5000, 500, 100); // 300~5000ms, 초기값 500ms
  yellowSlider.position(10, 120);
  yellowSlider.input(sendYellowTime);

  // 초록불 슬라이더
  createP("Green Time (ms)").position(10, 140);
  greenSlider = createSlider(300, 5000, 2000, 100); // 300~5000ms, 초기값 2000ms
  greenSlider.position(10, 170);
  greenSlider.input(sendGreenTime);
}

function draw() {
  background(220);
  push();
  translate(250, 50);

  // 빨간불 원: redOn이 true이면 빨간색, 아니면 회색
  if (redOn) fill(255, 0, 0);
  else fill(100);
  circle(0, 0, 50);

  // 노란불 원: yellowOn이 true이면 노란색, 아니면 회색
  if (yellowOn) fill(255, 255, 0);
  else fill(100);
  circle(0, 70, 50);

  // 초록불 원: greenOn이 true이면 초록색, 아니면 회색
  if (greenOn) fill(0, 255, 0);
  else fill(100);
  circle(0, 140, 50);
  pop();

  // 캔버스 하단에 현재 밝기와 현재 모드를 텍스트로 표시
  textSize(16);
  fill(0);
  text("brightness: " + brightnessVal, 10, height - 20);
  text("mode: " + currentMode, 10, height - 50);
}

async function connectToSerial() {
  try { // 연결할 포트 선택 
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });
    writer = port.writable.getWriter();
    reader = port.readable.pipeThrough(new TextDecoderStream()).getReader();

    // 성공할 경우 버튼 텍스트 변경
    connectButton.html("Connected!");
    console.log("Serial connected!");

    // 데이터 수신 시작
    readSerialData();

    // 접속 직후 슬라이더 값 전송
    sendRedTime();
    sendYellowTime();
    sendGreenTime();
  } catch (err) { // 에러러
    console.error("Serial Connection Error:", err);
  }
}

// 빨간불 시간 값 전송
function sendRedTime() {
  if (!writer) return;
  let val = redSlider.value();
  let msg = "RED " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

// 노란불 시간 값 전송
function sendYellowTime() {
  if (!writer) return;
  let val = yellowSlider.value();
  let msg = "YELLOW " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

// 초록불 시간 값 전송
function sendGreenTime() {
  if (!writer) return;
  let val = greenSlider.value();
  let msg = "GREEN " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

// 시리얼 데이터 수신
async function readSerialData() {
  let buffer = "";
  while (true) {
    const { value, done } = await reader.read();
    if (done) {
      reader.releaseLock();
      break;
    }
    if (value) {
      buffer += value;
      let lines = buffer.split("\n");
      // 마지막 줄은 완전하지 않을 수 있으므로 버퍼에 남겨둠
      buffer = lines.pop();
      for (let line of lines) {
        handleSerialLine(line.trim());
      }
    }
  }
}

// 시리얼로 수신한 각 줄을 처리하는 함수
function handleSerialLine(line) {
  console.log("Arduino says:", line);
  if (line.startsWith("STATE ")) { // STATE 메시지 처리
    let states = line.substring(6).split(",");
    redOn = (states[0] === "1");
    yellowOn = (states[1] === "1");
    greenOn = (states[2] === "1");
  } else if (line.startsWith("BRIGHTNESS ")) { // 밝기값 업데이트
    brightnessVal = parseInt(line.substring(11));
  } else if (line.startsWith("MODE ")) { // 모드 업데이트
    currentMode = line.substring(5).trim();
  } else {
    // MODE 메시지가 아닌 경우, 특정 키워드에 따라 처리
    if (line.indexOf("Manual Mode") !== -1) { // 1번버튼 눌리면
      if (line.indexOf("Off") !== -1) { // 끝나면 신호등 모드
        currentMode = "신호등";
      } else {
        currentMode = "Emergency (버튼1)"; // 눌리면 비상등 모드
      }
    } else if (line.indexOf("ALL_ON") !== -1 || line.indexOf("ALL_OFF") !== -1) {
      // 2번 버튼 눌리면
      if (!isBlinking) {
        isBlinking = true;
        currentMode = "Blinking (버튼2)";
      }
    } else if (line.indexOf("realOFFF") !== -1) {
      // 끝나면 신호등 모드
      isBlinking = false;
      currentMode = "신호등";
    } else if (line.indexOf("OFFWhite") !== -1) { // 3번 버튼 눌리면면
      currentMode = "On/Off (버튼3)"; // on/off 모드
    } else if (line.indexOf("ONNNN") !== -1) { // 끝나면 신호등 모드
      currentMode = "신호등";
    }
  }
}
