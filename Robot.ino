#define DEBUG true
#define VERSION 1.0
#include<TimerOne.h>

/* Robot Classes */
class Bridge{
    public:
        int A;
        int B;
        Bridge(int a,int b){
            A=a;
            B=b;
        }
       Bridge(){
            A=0;
            B=0;
       }
};
Bridge left(2,3),right(4,5);
class Robot{
    public: 
            Bridge left,right;
            Robot(Bridge a,Bridge b){
            left=a;
            right=b;
         }
        void go_forward(){
            digitalWrite(left.A,1);
            digitalWrite(left.B,0);
            digitalWrite(right.A,1);
            digitalWrite(right.B,0);
        }
        void go_backward(){
            digitalWrite(left.A,0);
            digitalWrite(left.B,1);
            digitalWrite(right.A,0);
            digitalWrite(right.B,1);
        }
        void stop(){
            digitalWrite(left.A,0);
            digitalWrite(left.B,0);
            digitalWrite(right.A,0);
            digitalWrite(right.B,0);
        }
        void go_left(){
            digitalWrite(right.A,0);    //c1 right c2//left
            digitalWrite(right.B,1);
            digitalWrite(left.A,1);
            digitalWrite(left.B,0);
        }
        void go_right(){
            digitalWrite(right.A,1);    //c1 right c2//left
            digitalWrite(right.B,0);
            digitalWrite(left.A,0);
            digitalWrite(left.B,1);
        }
};
/* END Robot Class */


/* Global Variables */
static int BAUDRATE=9600;
bool switch1=false;
long int time1=millis(); 
bool override=false;
Robot car(left,right);
static int ir_left=9,ir_right=8;
int distance=100;
bool irLeft, irRight;
static int ultrasonicThreshold=15;
static int trigger =10; 
static int echo = 12;
static int interruptTimeMillis=150000;
bool interruptSwitch;
int counter=0;
bool ultrasonicBack=false; // Ultrasonic is going back.
long int timer2;
/* END Global Variables */

void customISR(){
    interruptSwitch=!interruptSwitch;
}

void debugInterrupt(){
    printDebug("ISR:"+String(interruptSwitch));
}

/* Main Setup Function */
void setup() {
    pinMode(left.A,OUTPUT);
    pinMode(left.B,OUTPUT);
    pinMode(right.A,OUTPUT);
    pinMode(right.B,OUTPUT);
    pinMode(13,1);
    Serial.begin(BAUDRATE);
    Timer1.initialize(interruptTimeMillis);
    Timer1.attachInterrupt(customISR);
    time1=millis();
}
/* End Main Setup Funtion */

void ultrasonic_distance(){
    if(counter==0){
        digitalWrite(trigger,1);
    //     delayMicroseconds(10000);
        digitalWrite(trigger,0);
        distance= pulseIn(echo,HIGH)*0.034/2;
        counter++;
    }
    else{
        counter++;
    }
    if(counter==10){
        counter=0;
    }
    printDebug("Counter:"+String(counter));
}

void LDR(){
    int value_read=analogRead(A0);
}

/* Debug Function */
void debugStart(){
    if(DEBUG)
        Serial.print("Debug: ");
}
void printDebug(String data){
    if(DEBUG){
    Serial.print("\t"+data);
    }
}
void debugEnd(){
    if(DEBUG){
        Serial.println();
    }
}
void printDebug(int data){
    printDebug(String(data));
}
void printDebug(bool data){
    printDebug(String(data));
}
/* End debug function */

void ultrasonic(){
    if(distance<ultrasonicThreshold){
        ultrasonicBack=true;
        printDebug("US:"+ String(distance)+" uB:"+String(ultrasonicBack));
    }
    else{
        ultrasonicBack=false;
        printDebug("US:"+ String(distance)+" uB:"+String(ultrasonicBack));
    }
}
void IR(){  car.go_forward();
    irRight=digitalRead(ir_right);
    irLeft=digitalRead(ir_left);
    printDebug("PROXIMITY DATA: LEFT "+String(irLeft)+" RIGHT "+String(irRight));
}

/* Line follower function */
void lineFollower(){
    //Line follower has to link all boolean variables togather
    /*
     * Pending work...
     * 1) Ultrasonic module data not linked... (Crash =1 no crash=0)
     * 2) Override should be the master control of everything
     * 3) Panic button is needed to stop this robot...
     * 4) Interrupt is there, we have to co-work it with all the other variables.
     */
    digitalWrite(13,digitalRead(ir_left));
    if(irLeft) {
        car.go_left();
        printDebug(String("LF:IR_L E"));
   }
   else{
     car.go_forward();     
     printDebug(String("LF:IR_L D"));
   }
   if(irRight){
     car.go_right();
     printDebug(String("LR:IR_R E"));
   }
   else {
     car.go_forward();
     printDebug(String("LR:IR_R D"));
   }
}
/* END Line Foller Function */

/* Command Processor */
void commandProcessor(){ // Manual Override
    if(Serial.available()){
        Serial.setTimeout(100);
        String input = Serial.readString();
        if(input=="B\n"){
            car.go_backward(); 
        }
        else if(input=="F\n"){
            car.go_forward();
        }
        else if(input=="L\n"){
            car.go_left();
        }
        else if(input=="R\n"){
            car.go_right(); 
        }
        else if(input=="S\n"){
            car.stop(); 
        }
        else if(input=="O\n"){
            override=!override;
            Serial.println(override);
        }
        else{
            Serial.println("Meh");
            car.stop();
            override=false;
        }
    }
}

void loop(){
    debugStart();
    commandProcessor();
    interrupts();
    IR();
    lineFollower();
    LDR();
    ultrasonic_distance();
    ultrasonic();
    noInterrupts();
    debugInterrupt();
    debugEnd();
}
