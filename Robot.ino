#define DEBUG true
#define VERSION 1.0
#include <TimerOne.h>          // Interrupt handler ( Atmega Timer 1 )
#include <Wire.h>              // For I2C communication (LCD)
#include <LiquidCrystal_I2C.h> // For LCD
/* Pin Declarations */
#define BRIDGE_LEFT_A       2
#define BRIDGE_LEFT_B       3
#define BRIDGE_RIGHT_A      4
#define BRIDGE_RIGHT_B      5
#define ir_right            8
#define ir_left             9
#define trigger             10
#define echo                12
#define ldrPin              14
/* END Pin Declarations */

/* Global Declarations */
#define BAUDRATE            9600
#define ultrasonicThreshold 15
#define interruptTimeMillis 150000
/* END Global Declarations */

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
Bridge leftBridge(BRIDGE_LEFT_A,BRIDGE_LEFT_B),rightBridge(BRIDGE_RIGHT_A,BRIDGE_LEFT_B);
class Robot{
    public: 
            Bridge leftBridge,rightBridge;
            Robot(Bridge a,Bridge b){
            leftBridge=a;
            rightBridge=b;
         }
        void go_forward(){
            digitalWrite(leftBridge.A,1);
            digitalWrite(leftBridge.B,0);
            digitalWrite(rightBridge.A,1);
            digitalWrite(rightBridge.B,0);
        }
        void go_backward(){
            digitalWrite(leftBridge.A,0);
            digitalWrite(leftBridge.B,1);
            digitalWrite(rightBridge.A,0);
            digitalWrite(rightBridge.B,1);
        }
        void stop(){
            digitalWrite(leftBridge.A,0);
            digitalWrite(leftBridge.B,0);
            digitalWrite(rightBridge.A,0);
            digitalWrite(rightBridge.B,0);
        }
        void go_left(){
            digitalWrite(rightBridge.A,0);    //c1 right c2//left
            digitalWrite(rightBridge.B,1);
            digitalWrite(leftBridge.A,1);
            digitalWrite(leftBridge.B,0);
        }
        void go_right(){
            digitalWrite(rightBridge.A,1);    //c1 right c2//left
            digitalWrite(rightBridge.B,0);
            digitalWrite(leftBridge.A,0);
            digitalWrite(leftBridge.B,1);
        }
};
/* END Robot Class */


/* Global Variables */
Robot car(leftBridge,rightBridge);
LiquidCrystal_I2C	lcd(0x27,2,1,0,4,5,6,7);
long int time1=millis();   // Time Keeper ONE
bool override=true;        // Manual Override
int distance=100;          // Ultrasonic distance
bool irLeft, irRight;      // IR DATA
bool interruptSwitch;      // ISR ENABLE / DISABLE car
int counter=0;             // Ultrasonic Burst counter
bool ultrasonicBack=false; // Ultrasonic is going back.
long int timer2;           // Time Keeper TWO
int ldrData=1024;          // LDR DATA
bool ldrOK=false;          // LDR Light detect = 1
/* END Global Variables */

void customISR(){
    interruptSwitch=!interruptSwitch;
}

void debugInterrupt(){
    printDebug("ISR:"+String(interruptSwitch));
}

/* Main Setup Function */
void setup() {
    lcd.begin (16,2); // for 16 x 2 LCD module
    lcd.setBacklightPin(3,POSITIVE);
    lcd.setBacklight(HIGH);
    pinMode(leftBridge.A,OUTPUT);
    pinMode(leftBridge.B,OUTPUT);
    pinMode(rightBridge.A,OUTPUT);
    pinMode(rightBridge.B,OUTPUT);
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

void LDRData(){
    ldrData=analogRead(ldrPin);
}
void LDR(){
    if(ldrData>500){
        ldrOK=true;
    }
    else{
        ldrOK=false;
    }
}
/* Debug Function */
void debugStart(){
    if(DEBUG)
        Serial.print(F("Debug: "));
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

/* Line follower function new */

void lineFollowerNew(){
    //Line follower has to link all boolean variables togather
    /*
     * Pending work...
     * 1) Ultrasonic module data not linked... (Crash =1 no crash=0)
     * 2) Override should be the master control of everything
     * 3) Panic button is needed to stop this robot...
     * 4) Interrupt is there, we have to co-work it with all the other variables.
     */
    if(override){
        return;
    }
    if(interruptSwitch){
        car.stop();
        IR();
        return;
    }
    if(ultrasonicBack){
        car.go_backward();
        return;
    }
    digitalWrite(13,digitalRead(ir_left));
    if(irLeft) {
        car.go_left();
        printDebug(String(F("LF:IR_L E")));
   }
   else{
     car.go_forward();     
     printDebug(String(F("LF:IR_L D")));
   }
   if(irRight){
     car.go_right();
     printDebug(String(F("LR:IR_R E")));
   }
   else {
     car.go_forward();
     printDebug(String(F("LR:IR_R D")));
   }
}
/* END Line Follower Function new */

void LCD(){
    lcd.home();
    lcd.print(F("Hello World!"));
}
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
    LDRData();
    LDR();
    ultrasonic_distance();
    ultrasonic();
    lineFollowerNew();
    noInterrupts();
    debugInterrupt();
    debugEnd();
}
