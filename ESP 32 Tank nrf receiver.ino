#include <SPI.h>
#include <RF24.h>

// NRF24L01 Pins (CE, CSN)
RF24 radio(4, 5); 
const byte addr[6] = "00001";

// ===== Motor Pins =====
const int L_Fwd_Pin = 25; 
const int L_Rev_Pin = 26; 
const int R_Fwd_Pin = 32; 
const int R_Rev_Pin = 33; 

// PWM Settings
const int freq = 5000;
const int resolution = 8; // 0-255 range

struct __attribute__((packed)) ControlData {
  float L; 
  float R; 
};

ControlData ctrl;
unsigned long lastReceive = 0;
const unsigned long timeoutMs = 500;

void setup() {
  Serial.begin(115200);

  // --- NEW ESP32 PWM SETUP (v3.0 compatible) ---
  // Naye version mein ledcSetup ki zaroorat nahi hoti
  ledcAttach(L_Fwd_Pin, freq, resolution);
  ledcAttach(L_Rev_Pin, freq, resolution);
  ledcAttach(R_Fwd_Pin, freq, resolution);
  ledcAttach(R_Rev_Pin, freq, resolution);

  // --- Radio Setup ---
  if (!radio.begin()) {
    Serial.println("NRF24L01 hardware not responding!");
    while (1); // Stop if radio not found
  }
  
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_LOW);
  radio.openReadingPipe(1, addr);
  radio.startListening();

  Serial.println("Receiver Ready...");
}

void loop() {
  if (radio.available()) {
    radio.read(&ctrl, sizeof(ctrl));
    lastReceive = millis();
  }

  // Failsafe
  if (millis() - lastReceive > timeoutMs) {
    ctrl.L = 0;
    ctrl.R = 0;
  }

  controlMotors();
}

void controlMotors() {
  // Mapping and Deadband
  int pwm_L = map(abs((int)ctrl.L), 0, 100, 0, 255);
  int pwm_R = map(abs((int)ctrl.R), 0, 100, 0, 255);

  if (pwm_L < 15) pwm_L = 0;
  if (pwm_R < 15) pwm_R = 0;

  // Left Motor Logic
  if (ctrl.L > 0) {
    ledcWrite(L_Fwd_Pin, pwm_L);
    ledcWrite(L_Rev_Pin, 0);
  } else if (ctrl.L < 0) {
    ledcWrite(L_Fwd_Pin, 0);
    ledcWrite(L_Rev_Pin, pwm_L);
  } else {
    ledcWrite(L_Fwd_Pin, 0);
    ledcWrite(L_Rev_Pin, 0);
  }

  // Right Motor Logic
  if (ctrl.R > 0) {
    ledcWrite(R_Fwd_Pin, pwm_R);
    ledcWrite(R_Rev_Pin, 0);
  } else if (ctrl.R < 0) {
    ledcWrite(R_Fwd_Pin, 0);
    ledcWrite(R_Rev_Pin, pwm_R);
  } else {
    ledcWrite(R_Fwd_Pin, 0);
    ledcWrite(R_Rev_Pin, 0);
  }
}
