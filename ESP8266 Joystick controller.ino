#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(D2, D1);
const byte addr[6] = "00001";
ESP8266WebServer server(80);

const char* ssid = "ESP-REMOTE";
const char* password = "12345678";

struct __attribute__((packed)) ControlData {
  float L; 
  float R; 
};

ControlData ctrl;
unsigned long lastSend = 0;
bool lastSendStatus = false;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  radio.begin();
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(addr);
  radio.stopListening();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  if (millis() - lastSend > 30) {
    lastSendStatus = radio.write(&ctrl, sizeof(ctrl));
    lastSend = millis();
  }
}

void handleData() {
  if (server.hasArg("l")) ctrl.L = server.arg("l").toFloat();
  if (server.hasArg("r")) ctrl.R = server.arg("r").toFloat();
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  server.send(200, "text/plain", lastSendStatus ? "CONNECTED" : "ERROR");
}

void handleRoot() {
String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<style>
body { font-family:sans-serif; text-align:center; margin:0; touch-action:none; background:#1a1a1a; color:white; overflow:hidden;}

/* Single Clean Box */
#statusBox { 
  width:180px; margin:30px auto 10px auto; padding:15px; 
  border-radius:10px; background:#000; color:#0f0; 
  font-family:monospace; font-size:18px; font-weight:bold; 
  text-align:left; white-space:pre; border: 2px solid #333; 
}

.joy-container { width: 280px; height: 280px; background: radial-gradient(#444, #222); border-radius: 50%; position: relative; margin: 40px auto; border: 4px solid #333; box-shadow: inset 0 0 25px #000; }
.handle { width: 90px; height: 90px; background: linear-gradient(145deg, #00ff88, #009955); border-radius: 50%; position: absolute; top: 95px; left: 95px; box-shadow: 0 8px 15px rgba(0,0,0,0.4); }
</style>
</head>
<body>

<h3>PRO-TANK REMOTE</h3>

<div id="statusBox">WAITING...</div>

<div class="joy-container" id="joyZone"><div class="handle" id="stick"></div></div>

<script>
let valL=0, valR=0;
let lastSendTime=0, activeTouch=null;
let connStatus = "WAITING...";

const zone = document.getElementById('joyZone');
const stick = document.getElementById('stick');
const statusBox = document.getElementById('statusBox');

function updateUI() {
  statusBox.innerText = 
    connStatus + "\n\n" +
    "L : " + valL + "\n" +
    "R : " + valR;
}

function send(){
  let now = Date.now();
  if(now - lastSendTime < 50) return;
  lastSendTime = now;
  fetch(`/data?l=${valL}&r=${valR}`);
}

zone.addEventListener("touchstart", e => { 
  if(!activeTouch) { activeTouch = e.changedTouches[0].identifier; stick.style.transition="none"; } 
});

zone.addEventListener("touchmove", e => {
  e.preventDefault();
  let rect = zone.getBoundingClientRect(), cx = rect.width/2, cy = rect.height/2;
  for(let t of e.touches) {
    if(t.identifier === activeTouch) {
      let dx = t.clientX - rect.left - cx, dy = t.clientY - rect.top - cy;
      let maxDist = 95, dist = Math.sqrt(dx*dx + dy*dy);
      if(dist > maxDist) { dx *= maxDist/dist; dy *= maxDist/dist; }
      
      stick.style.transform = `translate(${dx}px, ${dy}px)`;

      let valX = Math.round((dx / maxDist) * 100);
      let valY = Math.round((-dy / maxDist) * 100);

      let rawL = valY + valX;
      let rawR = valY - valX;

      valL = Math.round(Math.max(-100, Math.min(100, rawL)));
      valR = Math.round(Math.max(-100, Math.min(100, rawR)));
      
      updateUI();
      send();
    }
  }
}, {passive:false});

zone.addEventListener("touchend", e => {
  for(let t of e.changedTouches) {
    if(t.identifier === activeTouch) {
      activeTouch = null;
      stick.style.transition = "transform 0.15s cubic-bezier(0.18, 0.89, 0.32, 1.28)";
      stick.style.transform = "translate(0px, 0px)";
      valL = 0; valR = 0;
      updateUI();
      send();
    }
  }
});

setInterval(() => { 
  fetch("/status").then(r=>r.text()).then(t => {
    connStatus = t;
    updateUI();
  }); 
}, 800);

</script>
</body>
</html>
)rawliteral";
server.send(200,"text/html",html);
}
