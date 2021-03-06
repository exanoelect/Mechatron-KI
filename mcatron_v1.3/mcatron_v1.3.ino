/*
  author  : dhikihandika 
  email   : dhikihandika36@gmail.com 
  date    : 02/07/2020 
*/

// #define DEBUG // Serial DEBUG
#define DEBUG_MOTOR // StepperMotor DEBUG

#define STR 10 //PIN -PushButton Start
#define STP 11 //PIN -PushButton Stop
#define LMS_T 8 //PIN -Top LimitSwitch
#define LMS_B 9 //PIN -Bottom LimitSwitch
#define DET 2 //PIN - Detector (IR Sensor) ~ optional use if need
#define OPR 6 //PIN - LED indicator operation
#define LSR 7 //PIN -Lasser
#define BZZ 12 //PIN -Buzzer

// #define EN 4 //PIN -Enable   
#define DIR 4 //PIN -Direction
#define STEP 5 //PIN -Pulse Step 
#define UP 1 //Move UP stepper motor
#define DOWN 0 //Move Down stepper motor
#define ON 0 //DO active mode
#define OFF 1 //DO OFF mode

enum STATE_PROCESS{
  STATE_INIT,
  STATE_PLACE_IN_UP,
  STATE_ADJUST_POSITION,
  STATE_AFTER_ADJUST
};

unsigned long crntMils=0;unsigned long crntMils1=0;unsigned long crntMils2=0;
unsigned long prevMils=0;unsigned long prevMils1=0;unsigned long prevMils2=0;
uint8_t state_process=STATE_INIT; //initial procces system

uint16_t MCRSTP=100; //Microstep declaration 
uint16_t PGEN=16000; //Pulse generate declaration
uint16_t cnt=1;uint16_t hth=1; //Buffer count
uint16_t i=1; //BUffer for loop condition

uint16_t clock=0; 
uint16_t clock1=0;
uint16_t clock2=0;

bool pbStp=false;int pbStr=0;
bool lmsTop=false;bool lmsBot=false;
bool dtctr=false;

//==============================//
/* procedure move motor stepper */
//==============================//
void stpMov(){
  for(i=1;i<=PGEN;i++){
    if(hth>=320){
      hth=0;
      MCRSTP=MCRSTP-1;
    }
    #ifdef DEBUG_MOTOR
    digitalWrite(STEP,1); delayMicroseconds(MCRSTP); // dutyCycle its 50%
    digitalWrite(STEP,0); delayMicroseconds(MCRSTP);
    #endif
    #ifdef DEBUG
    Serial.print(MCRSTP);Serial.print(" | ");Serial.print(i);Serial.print(" | ");Serial.println(cnt);
    #endif
    if(cnt>=PGEN){
      MCRSTP=50;
      // readPot();
      cnt=PGEN;
      break;
    }
    cnt++;hth++;
  }
}

//=========================//
/* procedure read value DI */
//=========================//
void rdLmsT(){
  int r=digitalRead(LMS_T);
   if(r==0){
    lmsTop=true;
  }
}
void rdLmsB(){
  int r=digitalRead(LMS_B);
  if(r==0){
    if(state_process==STATE_ADJUST_POSITION){
    lmsBot=true;
    }
  }
}
void detGo(){
  int r=digitalRead(DET);
  if(r==0){
    if(state_process==STATE_ADJUST_POSITION){
    dtctr=true;
    }
  }
}
void strRun(){
  int r=digitalRead(STR);
  if(r==0){
    pbStr=true;
    MCRSTP=100;PGEN=16000;cnt=1;i=1;
  }
}
void stpRun(){
  int r=digitalRead(STP);
  if(r==0){
    pbStp=true;
  }
}

//========================//
/* procedure millis clock */
//========================//
void millisClock(){
  crntMils=millis();
  if((crntMils-prevMils>=1000)&&(crntMils-prevMils<=1005)){
    clock++;
    prevMils=millis();
  }
}
void millisClock1(){
  crntMils1=millis();
  if((crntMils1-prevMils1>=1000)&&(crntMils1-prevMils1<=1005)){
    clock1++;
    prevMils1=millis();
  }
}
void millisClock2(){
  crntMils2=millis();
  if((crntMils2-prevMils2>=1000)&&(crntMils2-prevMils2<=1005)){
    clock2++;
    prevMils2=millis();
  }
}

//===============================//
/* Function use to SoftReset MCU */
//===============================//
void(* resetFunc) (void) = 0; //declare reset function @ address 0

//=======================//
/* Initilize the program */
//=======================//
void setup(){
  // initilize interface serial
  Serial.begin(9600);
  Serial.println("START!!!");

  // initilize digital I/O pin 
  pinMode(LMS_T,INPUT_PULLUP);  
  pinMode(LMS_B,INPUT_PULLUP);
  pinMode(STR,INPUT_PULLUP);
  pinMode(STP,INPUT_PULLUP);
  pinMode(DET,INPUT_PULLUP);

  pinMode(STEP,OUTPUT); 
  pinMode(DIR,OUTPUT);  
  // pinMode(EN, OUTPUT);
  pinMode(BZZ,OUTPUT);
  pinMode(LSR,OUTPUT);
  pinMode(OPR,OUTPUT);
 
  // setup state proccess
  state_process=STATE_INIT;
}

//===================//
/* main loop program */
//===================//
void loop() {
  switch(state_process){
  case STATE_INIT:
    millisClock();
    lmsBot=pbStr=pbStp=false;//Disable all variable can't use
    if(clock>=5){
      if(lmsTop){
        digitalWrite(OPR,OFF);
        state_process=STATE_PLACE_IN_UP;
        clock1=0;prevMils1=millis();
      } else {
        rdLmsT();
        digitalWrite(BZZ,OFF);digitalWrite(OPR,ON);digitalWrite(DIR,UP);stpMov();
      }
    } else {
      MCRSTP=100;PGEN=16000;cnt=1;i=1;
      digitalWrite(BZZ,ON);digitalWrite(OPR,OFF);digitalWrite(LSR, OFF);
    }
    break;
  case STATE_PLACE_IN_UP:
    lmsTop=lmsBot=pbStp=false;//Disable all variable can't use
    millisClock1();
    if(clock1>=5){
      strRun();
      if(pbStr){  //Problem in here
        state_process=STATE_ADJUST_POSITION;
        lmsBot=false;
      } else {
        digitalWrite(BZZ,OFF);
      }
    } else {
      digitalWrite(BZZ,ON);
    }
    break;
  case STATE_ADJUST_POSITION:
    lmsTop=pbStr=pbStp=false;//Disable all variable can't use
    if(lmsBot){
      digitalWrite(OPR,OFF);
      state_process=STATE_AFTER_ADJUST;prevMils2=millis();clock2=0;
    } else {
      rdLmsB();
      digitalWrite(LSR,ON);digitalWrite(OPR,ON);digitalWrite(DIR,DOWN);stpMov();
    }
    break;
  case STATE_AFTER_ADJUST:
    lmsTop=lmsBot=pbStr=false;
    millisClock2();
    if(clock2>=5){
      digitalWrite(BZZ,OFF);stpRun();
      if(pbStp){
        /* When manual false all programe */
        // lmsTop=lmsBot=pbStp=pbStp=false;
        // state_process=STATE_INIT;
        // digitalWrite(LSR,OFF);clock=0;prevMils=millis();

        /* When use softFunc */
        resetFunc();
      }
    } else {
      digitalWrite(BZZ,ON);
    }
    break;
  default:
    break;
  }
}