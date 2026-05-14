#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(D2, D1);
const byte addr[6] = "00001";
ESP8266WebServer server(80);

const char* ssid = "ESP-REMOTE";
const char* password = "12345678";

// Control structure - (Cleaned up, no auto-stabilize)
struct __attribute__((packed)) ControlData {
  float L; // -100 to 100
  float R; // -100 to 100
};

ControlData ctrl;

unsigned long lastSend = 0;
bool lastSendStatus = false;

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  delay(100);

  radio.begin();
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.setRetries(2, 3);
  radio.openWritingPipe(addr);
  radio.stopListening();

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/status", handleStatus);
  server.begin();
}

void loop() {
  server.handleClient();
  
  if (millis() - lastSend > 25) {
    lastSendStatus = radio.write(&ctrl, sizeof(ctrl));
    if (!lastSendStatus) radio.flush_tx();
    lastSend = millis();
  }
}

void handleData() {
  if (server.hasArg("l")) ctrl.L = server.arg("l").toFloat();
  if (server.hasArg("r")) ctrl.R = server.arg("r").toFloat();
  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  String msg;
  msg += lastSendStatus ? "CONNECTED\n" : "ERROR\n";
  msg += "L: " + String(ctrl.L) + "\n";
  msg += "R: " + String(ctrl.R) + "\n";
  server.send(200, "text/plain", msg);
}

void handleRoot() {
String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family:sans-serif; text-align:center; margin:0; touch-action:none; background:#222; color:white;}
.container { display:flex; justify-content:center; gap:20px; margin-top:40px; }
.joy { width:120px; height:300px; background:#ddd; position:relative; border-radius:20px; }
/* Handle default position set to middle (120px from top) */
.handle { width:100%; height:60px; background:#333; position:absolute; top:120px; border-radius:20px; transition: top 0.1s; }

#statusBox {
 width:200px; margin:auto; margin-top:20px; padding:10px;
 border-radius:10px; background:black; color:lime; font-size:14px;
 text-align:left; white-space:pre;
}
</style>
</head>
<body>

<h3>ESP Tank Controller</h3>
<div id="statusBox">WAIT...</div>

<div class="container">
<div class="joy" id="joyL"><div class="handle" id="hL"></div></div>
<div class="joy" id="joyR"><div class="handle" id="hR"></div></div>
</div>

<script>
let valL=0,valR=0,lastSend=0;

function sendData(){
 let now=Date.now();
 if(now-lastSend<80) return;
 lastSend=now;
 fetch(`/data?l=${valL}&r=${valR}`);
}

function setupJoystick(j,h,s){
 let id=null;

 j.addEventListener("touchstart",e=>{
  if(id===null) {
    id=e.changedTouches[0].identifier;
    h.style.transition = "none"; // Remove animation during drag
  }
 });

 j.addEventListener("touchmove",e=>{
  e.preventDefault();
  let r=j.getBoundingClientRect();

  for(let t of e.touches){
   if(t.identifier===id){
    let y=t.clientY-r.top;
    y=Math.max(0,Math.min(r.height,y));

    // Calculate -100 to 100 (Middle is 0)
    let percent = 1 - (y / r.height); 
    let value = Math.round((percent * 200) - 100);

    h.style.top = (y-30)+"px";

    if(s=="L") valL=value;
    else valR=value;

    sendData();
   }
  }
 },{passive:false});

 j.addEventListener("touchend",e=>{
  if(e.changedTouches[0].identifier===id) {
    id=null;
    h.style.transition = "top 0.2s"; // Add smooth snap back
    h.style.top = "120px"; // Snap to middle
    
    if(s=="L") valL=0;
    else valR=0;
    
    sendData();
  }
 });
}

setupJoystick(joyL,hL,"L");
setupJoystick(joyR,hR,"R");

setInterval(()=>{
 fetch("/status").then(r=>r.text()).then(t=>{
  statusBox.innerText = t;
 });
},800);
</script>
</body>
</html>
)rawliteral";
server.send(200,"text/html",html);
}
