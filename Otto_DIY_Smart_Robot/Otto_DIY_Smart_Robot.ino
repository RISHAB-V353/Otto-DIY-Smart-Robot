#include <Servo.h>
#include <Oscillator.h>

// ==========================================
// HARDWARE PINS & SETUP
// ==========================================
#define PIN_YL 5 // LeftLeg
#define PIN_YR 3 // RightLeg
#define PIN_RL 4 // LeftFoot
#define PIN_RR 2 // RightFoot
#define Buzzer 11
#define TrigPin A3
#define EchoPin A4

#define N_SERVOS 4
#define INTERVALTIME 10.0
#define DEG2RAD(g) ((g)*M_PI)/180

Oscillator servo[N_SERVOS];
int oldPosition[] = {90, 90, 90, 90};

unsigned long lastCommandTime = 0;
char currentCommand = 'S';
char lastExecutedCommand = 'S'; 
bool isAutonomous = false; 
bool isInterrupted = false;
bool manualSafety = true; // Controls obstacle detection in Manual Mode

// ==========================================
// CORE MOVEMENT ENGINE
// ==========================================

bool checkInterrupt() {
  if (Serial.available() > 0) {
    char peekChar = Serial.peek();
    if (peekChar != '\n' && peekChar != '\r') {
      isInterrupted = true;
      return true;
    } else {
      Serial.read(); 
    }
  }
  return false;
}

// Smart delay that listens for commands while waiting
void smartDelay(int ms) {
  unsigned long start = millis();
  while(millis() - start < ms) {
    if (checkInterrupt()) return;
    delay(1);
  }
}

#define CHK if(isInterrupted) return;

void oscillate(int A[N_SERVOS], int O[N_SERVOS], int T, double phase_diff[N_SERVOS]){
  for (int i=0; i<4; i++) {
    servo[i].SetO(O[i]);
    servo[i].SetA(A[i]);
    servo[i].SetT(T);
    servo[i].SetPh(phase_diff[i]);
  }
  double ref = millis();
  for (double x = ref; x < T + ref; x = millis()){
    if (checkInterrupt()) return; 
    for (int i=0; i<4; i++){ servo[i].refresh(); }
  }
}

void moveNServos(int time, int newPosition[]){
  float increment[N_SERVOS];
  for(int i=0; i<N_SERVOS; i++) increment[i] = ((newPosition[i]) - oldPosition[i]) / (time / INTERVALTIME);
  
  unsigned long final_time = millis() + time;
  int iteration = 1; 
  
  while(millis() < final_time){ 
    if (checkInterrupt()) return; 
    unsigned long interval_time = millis() + INTERVALTIME;
    int oneTime = 0;      
    while(millis() < interval_time){    
      if(oneTime < 1){ 
        for(int i=0; i<N_SERVOS; i++){
          servo[i].SetPosition(oldPosition[i] + (iteration * increment[i]));
        }     
        iteration++;
        oneTime++;
      }
    }     
  }   
  for(int i=0; i<N_SERVOS; i++) oldPosition[i] = newPosition[i];
}

void home() {
  int pos[4] = {90, 90, 90, 90};
  moveNServos(500, pos);
}

// ==========================================
// UNIFIED DANCE PRIMITIVES
// ==========================================

void walk(int steps, int T) {
  int A[4]= {15, 15, 30, 30}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void walkAuto(int steps, int T) {
  // Higher foot lift (25) exclusively for autonomous mode to prevent dragging/circling
  int A[4]= {25, 25, 30, 30}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void backyard(int steps, int T) {
  int A[4]= {15, 15, 30, 30}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(-90), DEG2RAD(-90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void turnLeft(int steps, int T) {
  int A[4]= {20, 20, 10, 30}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void turnRight(int steps, int T) {
  int A[4]= {20, 20, 30, 10}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void backTurnLeft(int steps, int T) {
  int A[4]= {20, 20, 10, 30}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(-90), DEG2RAD(-90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void backTurnRight(int steps, int T) {
  int A[4]= {20, 20, 30, 10}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(-90), DEG2RAD(-90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void moonWalkLeft(int steps, int T) {
  int A[4]= {25, 25, 0, 0}; int O[4] = {-15, 15, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(60), DEG2RAD(90), DEG2RAD(90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void moonWalkRight(int steps, int T) {
  int A[4]= {25, 25, 0, 0}; int O[4] = {-15, 15, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(300), DEG2RAD(90), DEG2RAD(90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void crusaito(int steps, int T) {
  int A[4]= {25, 25, 30, 30}; int O[4] = {-15, 15, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(300), DEG2RAD(90), DEG2RAD(90)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void twist(int steps, int tempo){
  int m1[4]={90,90,50,130}; int m2[4]={90,90,90,90};
  for(int x=0; x<steps; x++){ CHK
    moveNServos(tempo*0.5,m1); moveNServos(tempo*0.5,m2);
  }
}

void swing(int steps, int T) {
  int A[4]= {25, 25, 0, 0}; int O[4] = {-15, 15, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void upDown(int steps, int T) {
  int A[4]= {25, 25, 0, 0}; int O[4] = {-15, 15, 0, 0};
  double phase_diff[4] = {DEG2RAD(180), DEG2RAD(0), DEG2RAD(270), DEG2RAD(270)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void flapping(int steps, int T) {
  int A[4]= {15, 15, 8, 8}; int O[4] = {-A[0], A[1], 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(180), DEG2RAD(90), DEG2RAD(-90)};
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void run(int steps, int T) {
  int A[4]= {10, 10, 10, 10}; int O[4] = {0, 0, 0, 0};
  double phase_diff[4] = {DEG2RAD(0), DEG2RAD(0), DEG2RAD(90), DEG2RAD(90)}; 
  for(int i=0; i<steps; i++) { CHK oscillate(A,O, T, phase_diff); }
}

void lateral_fuerte(boolean side, int tempo){
  int m1[4]={90,90,90,90}; int m2[4]={90,90,90,90};
  if(side) m1[0]=40; else m1[1]=140;
  moveNServos(tempo/2, m1); CHK moveNServos(tempo/2, m2);
}

void drunk(int tempo){
  int m1[]={60,70,90,90};   int m2[]={110,120,90,90};
  int m3[]={60,70,90,90};   int m4[]={110,120,90,90};
  moveNServos(tempo*0.25,m1); CHK moveNServos(tempo*0.25,m2); CHK
  moveNServos(tempo*0.25,m3); CHK moveNServos(tempo*0.25,m4);
}

void kickLeft(int tempo){
  int m1[4]={50,60,90,90}; int m2[4]={80,60,90,90}; int m3[4]={40,60,90,90};
  moveNServos(tempo/4, m1); CHK moveNServos(tempo/4, m2); CHK
  moveNServos(tempo/4, m3); CHK moveNServos(tempo/4, m2);
}

void kickRight(int tempo){
  int m1[4]={120,130,90,90}; int m2[4]={120,100,90,90}; int m3[4]={120,140,90,90};
  moveNServos(tempo/4, m1); CHK moveNServos(tempo/4, m2); CHK
  moveNServos(tempo/4, m3); CHK moveNServos(tempo/4, m2);
}

void reverencia1(int steps, int tempo){
  int m1[4]={130,50,90,90}; int m2[4]={90,90,90,90};
  for(int x=0; x<steps; x++){ CHK moveNServos(tempo*0.5,m1); moveNServos(tempo*0.5,m2); }
}

void reverencia2(int steps, int tempo){
  int m1[4]={130,50,90,90}; int m2[4]={130,50,60,120}; int m3[4]={90,90,90,90};
  for(int x=0; x<steps; x++){ CHK
    moveNServos(tempo*0.3,m1); moveNServos(tempo*0.3,m2); moveNServos(tempo*0.4,m3);
  }
}

void saludo(int steps, int tempo){
  int m1[4]={60,60,90,90}; int m2[4]={120,60,90,90};
  for(int x=0; x<steps; x++){ CHK moveNServos(tempo*0.5,m1); moveNServos(tempo*0.5,m2); }
}

void pasitos(int steps, int tempo){
  int m1[4]={90,120,60,60}; int m2[4]={90,90,90,90};
  int m3[4]={60,90,120,120}; int m4[4]={90,90,90,90};
  for(int i=0; i<steps; i++){ CHK
    moveNServos(tempo*0.25,m1); moveNServos(tempo*0.25,m2);
    moveNServos(tempo*0.25,m3); moveNServos(tempo*0.25,m4);
  }
}

void patada(int tempo){
  int m1[4]={115,120,90,90}; int m2[4]={115,70,90,90};
  int m3[4]={100,80,90,90};  int m4[4]={90,90,90,90};
  moveNServos(tempo/4, m1); CHK moveNServos(tempo/4, m2); CHK
  moveNServos(tempo/4, m3); CHK moveNServos(tempo/4, m4);
}

void goingUp(int tempo){
  int m1[4]={80,100,90,90}; int m2[4]={60,120,90,90}; 
  int m3[4]={40,140,90,90}; int m4[4]={20,160,90,90};
  moveNServos(tempo/4,m1); CHK moveNServos(tempo/4,m2); CHK
  moveNServos(tempo/4,m3); CHK moveNServos(tempo/4,m4);
}

void noGravity(int tempo){
  int m1[4]={120,140,90,90}; int m2[4]={140,140,90,90}; 
  int m3[4]={120,140,90,90}; int m4[4]={90,90,90,90};
  moveNServos(tempo,m1); CHK moveNServos(tempo,m2); CHK
  moveNServos(tempo,m3); CHK moveNServos(tempo,m4);
}

void primera_parte(){
  int m1[4]={60,120,90,90}; int m2[4]={90,90,90,90}; int m3[4]={40,140,90,90};
  int t = 495;
  for(int x=0; x<3; x++){
    for(int i=0; i<3; i++){ lateral_fuerte(1,t/2); CHK lateral_fuerte(0,t/4); CHK }
    moveNServos(t*0.4,m1); CHK moveNServos(t*0.4,m2); CHK
  }
  for(int i=0; i<2; i++){ lateral_fuerte(1,t/2); CHK lateral_fuerte(0,t/4); CHK }
  crusaito(1,t*1.4); CHK moveNServos(t*1,m3);
}

void segunda_parte(){
  int m1[4]={90,90,80,100};  int m2[4]={90,90,100,80};
  int m5[4]={40,140,80,100}; int m6[4]={40,140,100,80};
  int t = 495;
  for(int x=0; x<7; x++){ 
    for(int i=0; i<3; i++){ moveNServos(t*0.15,m1); CHK moveNServos(t*0.15,m2); CHK }
    moveNServos(t*0.15,m5); CHK moveNServos(t*0.15,m6); CHK
  }
  for(int i=0; i<3; i++){ moveNServos(t*0.15,m5); CHK moveNServos(t*0.15,m6); CHK }
}

// ==========================================
// FULL MACRO CHOREOGRAPHIES
// ==========================================

void singleLadiesDance() {
  isInterrupted = false;
  int t = 620;
  pasitos(8,t*2); CHK crusaito(1,t); CHK patada(t); CHK twist(2,t); CHK
  twist(3,t/2); CHK upDown(1,t*2); CHK patada(t*2); CHK drunk(t*2); CHK
  flapping(1,t*2); CHK walk(2,t); CHK walk(1,t*2); CHK backyard(2,t); CHK
  patada(t*2); CHK flapping(1,t*2); CHK patada(t*2); CHK twist(8,t/2); CHK
  moonWalkLeft(2,t); CHK crusaito(1,t*2); CHK
  
  for(int i=0; i<2; i++){ lateral_fuerte(0,t); CHK lateral_fuerte(1,t); CHK upDown(1,t*2); CHK }
  saludo(1,t*2); CHK saludo(1,t); CHK swing(3,t); CHK
  
  lateral_fuerte(0,t); CHK lateral_fuerte(1,t); CHK lateral_fuerte(0,t/2); CHK lateral_fuerte(1,t/2); CHK
  pasitos(1,t*2); CHK pasitos(1,t); CHK pasitos(1,t*2); CHK pasitos(1,t); CHK
  
  crusaito(2,t); CHK crusaito(1,t*2); CHK crusaito(2,t); CHK crusaito(1,t*2); CHK
  upDown(2,t); CHK crusaito(1,t*2); CHK pasitos(2,t*2); CHK pasitos(2,t); CHK
  flapping(1,t*2); CHK upDown(2,t); CHK upDown(1,t*2); CHK
  
  for (int i=0; i<4; i++){ pasitos(1,t); CHK }
  reverencia1(1,t*4); CHK reverencia2(1,t*4); CHK upDown(1,t); CHK run(2,t/2); CHK patada(t*2); CHK
  lateral_fuerte(0,t); CHK lateral_fuerte(1,t); CHK upDown(2,t); CHK
  pasitos(4,t); CHK patada(t*2); CHK pasitos(2,t); CHK swing(2,t*2); CHK
  
  for (int i=0; i<4; i++){ lateral_fuerte(0,t); CHK lateral_fuerte(1,t); CHK }
  pasitos(6,t); CHK swing(4,t); CHK twist(2,t/2); CHK drunk(t*2); CHK 
  walk(1,t); CHK backyard(1,t); CHK crusaito(3,t); CHK upDown(1,t*2); CHK
  kickLeft(t/2); CHK kickRight(t/2); CHK moonWalkLeft(3,t); CHK moonWalkRight(3,t); CHK
  walk(4,t); CHK backyard(4,t); CHK lateral_fuerte(0,t); CHK lateral_fuerte(1,t); CHK
  
  for (int i=0; i<7; i++){ pasitos(2,t); CHK swing(2,t); CHK }
  crusaito(1,t*2); CHK upDown(1,t);
}

void smoothCriminalDance() {
  isInterrupted = false;
  int t = 495;
  primera_parte(); CHK segunda_parte(); CHK
  moonWalkLeft(4,t*2); CHK moonWalkRight(4,t*2); CHK moonWalkLeft(4,t*2); CHK moonWalkRight(4,t*2); CHK
  primera_parte(); CHK 
  crusaito(1,t*8); CHK crusaito(1,t*7); CHK
  for (int i=0; i<16; i++){ flapping(1,t/4); CHK }
  moonWalkRight(4,t*2); CHK moonWalkLeft(4,t*2); CHK
  drunk(t*4); CHK drunk(t*4); CHK kickLeft(t); CHK kickRight(t); CHK
  drunk(t*8); CHK walk(2,t*2); CHK backyard(2,t*2); CHK
  goingUp(t*2); CHK noGravity(t*2); CHK
  crusaito(1,t*2); CHK crusaito(1,t*8); CHK crusaito(1,t*2); CHK
  primera_parte(); CHK
  for (int i=0; i<32; i++){ flapping(1,t/2); CHK }
}

void marioDance() {
  isInterrupted = false;
  
  for(int i=0; i<3; i++) {
    tone(Buzzer, 988, 80); smartDelay(80); CHK
    tone(Buzzer, 1319, 200); 
    upDown(1, 400); CHK 
    smartDelay(100); CHK
  }
  noTone(Buzzer);
  
  tone(Buzzer, 1319, 100); smartDelay(100); CHK
  tone(Buzzer, 1568, 100); smartDelay(100); CHK
  tone(Buzzer, 2637, 100); smartDelay(100); CHK
  tone(Buzzer, 2093, 100); smartDelay(100); CHK
  tone(Buzzer, 2349, 100); smartDelay(100); CHK
  tone(Buzzer, 3136, 200); 
  twist(4, 500); CHK
  noTone(Buzzer);
}

void ninjaStrike() {
  isInterrupted = false;
  
  walk(2, 1200); CHK
  
  tone(Buzzer, 3000, 50); smartDelay(50); CHK noTone(Buzzer);
  kickLeft(400); CHK
  
  tone(Buzzer, 3000, 50); smartDelay(50); CHK noTone(Buzzer);
  kickRight(400); CHK
  
  turnLeft(4, 400); CHK
  
  reverencia1(1, 1000); CHK
}

void glitchMode() {
  isInterrupted = false;
  
  for(int i=0; i<6; i++) {
    tone(Buzzer, random(500, 3000), 50);
    lateral_fuerte(i%2, 150); CHK
  }
  noTone(Buzzer);
  
  drunk(400); CHK 
  drunk(400); CHK
  
  for(int i=2000; i>200; i-=50) {
    tone(Buzzer, i, 20); smartDelay(20); CHK
  }
  noTone(Buzzer);
  
  goingUp(800); CHK
}

void moonwalkSolo() {
  isInterrupted = false;
  
  for(int i=0; i<4; i++) {
    moonWalkLeft(2, 600); CHK
    moonWalkRight(2, 600); CHK
  }
  crusaito(2, 800); CHK
}

void trexRage() {
  isInterrupted = false;
  
  walk(4, 1200); CHK
  tone(Buzzer, 150, 1000); smartDelay(1000); CHK
  noTone(Buzzer);
  
  turnRight(6, 400); CHK
  backyard(4, 1200); CHK
}

// ==========================================
// OBSTACLE ALARM 
// ==========================================
void obstacleAlarm() {
  isInterrupted = false;
  for (int i = 0; i < 3; i++) {
    tone(Buzzer, 1200, 100); 
    smartDelay(100); CHK
    noTone(Buzzer);
    smartDelay(100); CHK
  }
}


// ==========================================
// MAIN ARDUINO LOGIC
// ==========================================

void setup() {
  servo[0].attach(PIN_RR); 
  servo[1].attach(PIN_RL); 
  servo[2].attach(PIN_YR); 
  servo[3].attach(PIN_YL); 
  
  // Seed the random generator with noise from an unused pin
  randomSeed(analogRead(A0)); 

  for(int i=0; i<4; i++) {
    servo[i].SetTrim(0); 
    servo[i].SetPosition(90);
  }
  
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  
  Serial.begin(9600); 

  // Startup Chime
  tone(Buzzer, 880, 100); delay(100); // A5
  tone(Buzzer, 988, 100); delay(100); // B5
  tone(Buzzer, 1319, 200); delay(200); // E6
}

int getDistance() {
  digitalWrite(TrigPin, LOW); delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH); delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  long duration = pulseIn(EchoPin, HIGH, 30000); 
  if (duration == 0) return 999; 
  return duration * 0.034 / 2; 
}

void loop() {
  if (Serial.available()) {
    char incoming = Serial.read();
    if (incoming != '\n' && incoming != '\r') {
      lastCommandTime = millis(); 
      isInterrupted = false; 

      // ==========================================
      // EMERGENCY STOP BUTTON ('X')
      // ==========================================
      if (incoming == 'X') {
        isAutonomous = false;
        currentCommand = 'S';
        isInterrupted = true; // Instantly breaks any running loops
        
        // E-Stop Warning Tone (Low double beep)
        tone(Buzzer, 200, 200); delay(200);
        tone(Buzzer, 150, 400); delay(400);
        
        home(); // Snap to attention
        return; // Skip the rest of this loop cycle immediately
      }

      if (incoming == 'A') {
        isAutonomous = true;
        // Happy ascending toggle tone
        tone(Buzzer, 1047, 100); delay(100); 
        tone(Buzzer, 1319, 100); delay(100); 
        tone(Buzzer, 1568, 100); delay(100); 
      } 
      else if (incoming == 'M') {
        isAutonomous = false;
        currentCommand = 'S'; 
        // Descending toggle tone
        tone(Buzzer, 1568, 100); delay(100); 
        tone(Buzzer, 1319, 100); delay(100); 
        tone(Buzzer, 1047, 100); delay(100); 
      } 
      else if (incoming == 'O') {
        manualSafety = true;
        tone(Buzzer, 2000, 100); // High beep to confirm Safety ON
      }
      else if (incoming == 'P') {
        manualSafety = false;
        tone(Buzzer, 500, 100); // Low warning beep for Safety OFF
      }
      else if (!isAutonomous) {
        currentCommand = incoming;
      }
    }
  }

  // 2. Autonomous Mode
  if (isAutonomous) {
    if (getDistance() < 15) {
      obstacleAlarm(); 
      isInterrupted = false; backyard(5, 600); // Step out of detection zone
      isInterrupted = false; 
      
      // Randomized Turn
      if (random(0, 2) == 0) {
        turnLeft(3, 1000);
      } else {
        turnRight(3, 1000);
      }
      
    } else {
      isInterrupted = false; walkAuto(1, 600);  // High-stepping autonomous walk
    }
  } 
  
  // 3. Manual Mode
  else {
    if (millis() - lastCommandTime > 1000 && currentCommand != 'S') {
      currentCommand = 'S';
    }

    if (currentCommand == 'F') {
      if (manualSafety && getDistance() < 15) { 
        obstacleAlarm(); 
        currentCommand = 'S';   
      } else {
        isInterrupted = false; walk(1, 600); 
      }
    } 
    else if (currentCommand == 'B') {
      isInterrupted = false; backyard(1, 600); 
    } 
    else if (currentCommand == 'L') {
      if (manualSafety && getDistance() < 15) {
        obstacleAlarm();
        currentCommand = 'S';
      } else {
        isInterrupted = false; turnLeft(1, 1000); 
      }
    } 
    else if (currentCommand == 'R') {
      if (manualSafety && getDistance() < 15) {
        obstacleAlarm();
        currentCommand = 'S';
      } else {
        isInterrupted = false; turnRight(1, 1000); 
      }
    } 
    else if (currentCommand == 'J') {
      isInterrupted = false; backTurnLeft(1, 1000); 
    }
    else if (currentCommand == 'K') {
      isInterrupted = false; backTurnRight(1, 1000); 
    }
    else if (currentCommand == 'S') {
      if (lastExecutedCommand != 'S') {
        home(); 
      }
    }
    
    // --- The Big Dances & Macros ---
    else if (currentCommand == '1') {
      singleLadiesDance();
      if (!isInterrupted) currentCommand = 'S'; 
    } 
    else if (currentCommand == '2') {
      smoothCriminalDance();
      if (!isInterrupted) currentCommand = 'S'; 
    } 
    else if (currentCommand == '3') {
      marioDance();
      if (!isInterrupted) currentCommand = 'S'; 
    }
    else if (currentCommand == '4') {
      ninjaStrike();
      if (!isInterrupted) currentCommand = 'S'; 
    }
    else if (currentCommand == '5') {
      glitchMode();
      if (!isInterrupted) currentCommand = 'S'; 
    }
    else if (currentCommand == '6') {
      moonwalkSolo();
      if (!isInterrupted) currentCommand = 'S'; 
    }
    else if (currentCommand == '7') {
      trexRage();
      if (!isInterrupted) currentCommand = 'S'; 
    }

    lastExecutedCommand = currentCommand; 
  }
}