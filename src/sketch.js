// 변수
let redOn = false;
let yellowOn = false;
let greenOn = false;
let brightnessVal = 0;  // Arduino에서 전송받은 밝기 값
let port;
let reader;
let writer;
let connectButton;
let redSlider, yellowSlider, greenSlider;

function setup() {
  createCanvas(400, 300);

  // 연결 버튼
  connectButton = createButton("Connect to Arduino");
  connectButton.position(10, 10);
  connectButton.mousePressed(connectToSerial);

  // 빨간불 슬라이더
  createP("Red Time (ms)").position(10, 40);
  redSlider = createSlider(300, 5000, 2000, 100);
  redSlider.position(10, 70);
  redSlider.input(sendRedTime);

  // 노란불 슬라이더
  createP("Yellow Time (ms)").position(10, 90);
  yellowSlider = createSlider(300, 5000, 500, 100);
  yellowSlider.position(10, 120);
  yellowSlider.input(sendYellowTime);

  // 초록불 슬라이더
  createP("Green Time (ms)").position(10, 140);
  greenSlider = createSlider(300, 5000, 2000, 100);
  greenSlider.position(10, 170);
  greenSlider.input(sendGreenTime);
}

function draw() {
  background(220);
  push();
  translate(250, 50);

  // 빨간불 원
  if (redOn) fill(255, 0, 0); // 불이 켜져있으면 빨간색
  else fill(100); // 꺼져있으면 회색
  circle(0, 0, 50);

  // 노란불 원
  if (yellowOn) fill(255, 255, 0); // 불이 켜져있으면 노란색
  else fill(100); // 꺼져있으면 회색
  circle(0, 70, 50);

  // 초록불 원
  if (greenOn) fill(0, 255, 0); // 불이 켜져있으면 초록색
  else fill(100); // 꺼져있으면 회색
  circle(0, 140, 50);
  pop();

  // p5 창 하단에 현재 밝기 표시
  textSize(16);
  fill(0);
  text("brightness: " + brightnessVal, 10, height - 20);
}

// 시리얼 연결
async function connectToSerial() {
  try {
    // 사용자에게 포트 선택 요청
    port = await navigator.serial.requestPort();
    await port.open({ baudRate: 9600 });
    writer = port.writable.getWriter();
    reader = port.readable.pipeThrough(new TextDecoderStream()).getReader();
    
    // 성공하면 연결됨!
    connectButton.html("Connected!");
    console.log("Serial connected!");

    // 데이터 수신 시작
    readSerialData();

    // 접속 직후 슬라이더 값 전송
    sendRedTime();
    sendYellowTime();
    sendGreenTime();
  } catch (err) { // 에러라면 에러 메시지
    console.error("Serial Connection Error:", err);
  }
}

// 값을 슬라이더에서 읽어서 전송
function sendRedTime() {
  if (!writer) return;
  let val = redSlider.value();
  let msg = "RED " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

function sendYellowTime() {
  if (!writer) return;
  let val = yellowSlider.value();
  let msg = "YELLOW " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

function sendGreenTime() {
  if (!writer) return;
  let val = greenSlider.value();
  let msg = "GREEN " + val + "\n";
  writer.write(new TextEncoder().encode(msg));
  console.log("Send:", msg.trim());
}

// 데이터 읽어서 분리하고 처리
async function readSerialData() {
  let buffer = ""; // 문자열 버퍼 초기화
  while (true) {
    const { value, done } = await reader.read(); // 데이터 읽기
    if (done) { // 읽기가 끝났다면 종료
      reader.releaseLock();
      break;
    }
    if (value) { // 읽었다면 기존 버퍼에 추가
      buffer += value;
      let lines = buffer.split("\n");
      buffer = lines.pop(); // 마지막 줄은 완전하지 않을 수 있으므로 버퍼에 남겨둠
      for (let line of lines) {
        handleSerialLine(line.trim()); // 공백 제거
      }
    }
  }
}

// 문자열 처리해서 동작시켜
function handleSerialLine(line) {
  console.log("Arduino says:", line);
  if (line.startsWith("STATE ")) { // state로 시작하면
    let states = line.substring(6).split(","); // 분리시켜
    redOn = (states[0] === "1"); // 첫번째가 1이라면 빨간불 켜
    yellowOn = (states[1] === "1"); // 두번째가 1이라면 노란불 켜
    greenOn = (states[2] === "1"); // 세번째가 1이라면 초록불 켜
  } else if (line.startsWith("BRIGHTNESS ")) { // BRIGHTNESS로 시작하면
    brightnessVal = parseInt(line.substring(11)); // 그 뒤 문자열을 숫자로
  } else {
    console.log("Other:", line);
  }
}
