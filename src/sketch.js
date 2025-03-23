// 신호등 LED 상태 및 시스템 변수
let redOn = false;
let yellowOn = false;
let greenOn = false;
let brightnessVal = 0;
let currentMode = "신호등";

// 시리얼 통신 관련 변수
let port, reader, writer;
let connectButton;

// 슬라이더 UI
let redSlider, yellowSlider, greenSlider;

// Handpose 관련 변수
let handPose;
let video;
let hands = [];
let camX = 500, camY = 0, camW = 640, camH = 480;

// 제스처 관련 변수
let lastGesture = null;
let gestureCooldown = 0;
let sliderGestureCooldown = 0;

function setup() {
  createCanvas(1200, 480);
  // 웹캠 및 Handpose 모델 초기화
  video = createCapture(VIDEO, { flipped: true });
  video.size(640, 480);
  video.hide();

  handPose = ml5.handpose(video, modelReady);
  handPose.on("predict", gotHands);

  // 시리얼 연결 버튼
  connectButton = createButton("Connect!");
  connectButton.position(20, 20);
  connectButton.mousePressed(connectToSerial);

  // 슬라이더 UI
  createP("Red Time (ms)").position(20, 50);
  redSlider = createSlider(300, 5000, 2000, 100);
  redSlider.position(20, 80);
  redSlider.input(sendRedTime);

  createP("Yellow Time (ms)").position(20, 110);
  yellowSlider = createSlider(300, 5000, 500, 100);
  yellowSlider.position(20, 140);
  yellowSlider.input(sendYellowTime);

  createP("Green Time (ms)").position(20, 170);
  greenSlider = createSlider(300, 5000, 2000, 100);
  greenSlider.position(20, 200);
  greenSlider.input(sendGreenTime);
}

function modelReady() {
  console.log("Handpose model ready!");
}

function draw() {
  background(220);

  // 웹캠 영상
  image(video, camX, camY, camW, camH);
  fill(255, 0, 0);
  noStroke();

  // 손 인식 결과 그리기
  for (let i = 0; i < hands.length; i++) {
    for (let [xVal, yVal] of hands[i].landmarks) {
      let flippedX = camX + (video.width - xVal);
      let drawY = camY + yVal;
      circle(flippedX, drawY, 10);
    }
  }

  // 신호등 그리기
  if (redOn) fill(255, 0, 0); else fill(100); circle(200, 70, 50);
  if (yellowOn) fill(255, 255, 0); else fill(100); circle(200, 140, 50);
  if (greenOn) fill(0, 255, 0); else fill(100); circle(200, 210, 50);
  fill(0); textSize(16);
  text("brightness: " + brightnessVal, 10, height - 20);
  text("mode: " + currentMode, 10, height - 50);

  // 제스처 처리
  if (hands.length > 0 && millis() > gestureCooldown) {
    let newMode = detectModeGesture(hands[0]);
    if (newMode && newMode !== currentMode) {
      gestureCooldown = millis() + 2000;
      currentMode = newMode;
      sendModeCommandMode(newMode);
      console.log("Mode changed via gesture:", newMode);
    }
  }

  // 슬라이더 제스처 인식
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
      } else if (isPalmUp(hand)) {                                                             // 손바닥을 위로 올리면
        let val = min(greenSlider.value() + 100, parseInt(greenSlider.elt.max));        // 초록색 시간을 늘림
        greenSlider.value(val); sendGreenTime();                                        // 시리얼로 전송      
        console.log("손바닥 위");                                                        // 콘솔에 로그 출력
      } 
      
      else {
        continue;
      }
      sliderGestureCooldown = millis() + 2000;                                          // 제스처 인식 쿨다운 2초로 설정
      break;
    }
  }
}

// 제스처 인식 함수
function detectModeGesture(hand) {                                                      
  const lm = hand.landmarks;                                                            // 손의 랜드마크 정보
  const isUp = (tip, dip) => lm[tip][1] < lm[dip][1];                                   // 손가락이 펴져있는지 확인하는 함수
  const idx = isUp(8, 6), mid = isUp(12, 10), rng = isUp(16, 14), pink = isUp(20, 18);  // 각 손가락의 상태를 저장
  if (!idx && !mid && !rng && !pink) return "NORMAL";                                   // 모든 손가락이 접혀있으면 NORMAL
  if (idx && !mid && !rng && !pink) return "EMERGENCY";                                 // 검지만 펴져있으면 EMERGENCY
  if (idx && mid && !rng && !pink) return "BLINKING";                                   // 검지와 중지만 펴져있으면 BLINKING
  if (idx && mid && rng && !pink) return "ON_OFF";                                      // 검지, 중지, 약지가 펴져있으면 ON_OFF
  return null;
}

// 시리얼 연결 함수
async function connectToSerial() {                                                      
  try {
    port = await navigator.serial.requestPort();                                        // 시리얼 포트 요청
    await port.open({ baudRate: 9600 });                                                // 시리얼 포트 오픈
    writer = port.writable.getWriter();                                                 // writer 객체 생성
    reader = port.readable.pipeThrough(new TextDecoderStream()).getReader();            // reader 객체 생성
    connectButton.html("Connected!");                                     
    readSerialData();                                                                   // 시리얼 데이터 읽기          
    sendRedTime(); sendYellowTime(); sendGreenTime();                                   // 빨간, 노란, 초록 시간 전송
  } catch (err) {
    console.error("Serial Connection Error:", err);
  }
}

// 시리얼 데이터 읽기 함수
async function readSerialData() {                                                       
  let buffer = "";
  while (true) {                                                                          
    const { value, done } = await reader.read();                                        // reader로부터 데이터 읽기
    if (done) break;                                                                    // 데이터가 없으면 종료
    if (value) {
      buffer += value;                                                                  // 버퍼에 데이터 추가
      let lines = buffer.split("\n");                                                   // 개행문자로 데이터 분리
      buffer = lines.pop();                                                             // 마지막 데이터는 버퍼에 저장   
      lines.forEach(line => handleSerialLine(line.trim()));                             // 각 데이터에 대해 처리
    }
  }
}

// 시리얼 데이터 처리 함수
function handleSerialLine(line) {                                                       
  console.log("Arduino says:", line);                                                 
  if (line.startsWith("STATE ")) {                                                      // STATE로 시작하는 데이터 처리     
    let [r, y, g] = line.substring(6).split(",");                                       // 빨간, 노란, 초록 LED 상태 저장
    redOn = (r === "1"); yellowOn = (y === "1"); greenOn = (g === "1");                   
  } else if (line.startsWith("BRIGHTNESS ")) {                                          // BRIGHTNESS로 시작하는 데이터 처리
    brightnessVal = parseInt(line.substring(11));                                       // 밝기 값 저장
  } else if (line.startsWith("MODE ")) {                                                // MODE로 시작하는 데이터 처리  
    currentMode = line.substring(5).trim();                                             // 모드 값 저장
  }
}

function sendRedTime() {                                                                // 빨간색 시간 전송 함수
  if (!writer) return;                                                                  // writer 객체가 없으면 종료
  writer.write(new TextEncoder().encode("RED " + redSlider.value() + "\n"));            // 빨간색 시간 전송
}
function sendYellowTime() {                                                             // 노란색 시간 전송 함수
  if (!writer) return;                
  writer.write(new TextEncoder().encode("YELLOW " + yellowSlider.value() + "\n"));      // 노란색 시간 전송
}
function sendGreenTime() {                                                              // 초록색 시간 전송 함수
  if (!writer) return;
  writer.write(new TextEncoder().encode("GREEN " + greenSlider.value() + "\n"));        // 초록색 시간 전송
}
function sendModeCommandMode(mode) {                                                    // 모드 전송 함수
  if (!writer) return;
  writer.write(new TextEncoder().encode(`MODE:${mode}\n`));                             // 모드 전송
}

// 손 인식 결과 저장 함수
function gotHands(results) { hands = results; }                                         

function isIndexandPinkyExtended(hand) {                                                // 검지와 새끼 위로로 핀 함수      
  let lm = hand.landmarks;
  return (lm[8][1] < lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] < lm[18][1]);
}
function isThumbandIndexExtended(hand) {                                                    // 검지와 새끼 아래로 핀 함수
  let lm = hand.landmarks;
  return (lm[16][1] > lm[14][1] && lm[12][1] > lm[10][1] && lm[8][1] < lm[6][1] && lm[20][1] > lm[18][1] && lm[4][1] < lm[3][1]);
}
function isThumbExtended(hand) {                                                        // 엄지 위로 핀 함수
  let lm = hand.landmarks;
  return (lm[4][1] < lm[3][1] && lm[8][1] > lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] < lm[18][1]);
}
function isThumbDown(hand) {                                                            // 엄지 아래로 핀 함수
  let lm = hand.landmarks;
  return lm[4][1] > lm[3][1];
}
function isPalmUp(hand) {                                                               // 손바닥 위로
  let lm = hand.landmarks;
  return (lm[4][1] < lm[3][1] && lm[8][1] < lm[6][1] && lm[12][1] < lm[10][1] && lm[16][1] < lm[14][1] && lm[20][1] < lm[18][1]);
}
function isPalmDown(hand) {                                                             // 손바닥 아래로
  let lm = hand.landmarks;
  return (lm[4][1] > lm[3][1] && lm[8][1] > lm[6][1] && lm[12][1] > lm[10][1] && lm[16][1] > lm[14][1] && lm[20][1] > lm[18][1]);
}
