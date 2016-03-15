
#include <pt.h>

#define Moter_1 11
#define Moter_2 10
#define Moter_3 9
#define Sleep 8

#define M1 7
#define M2 6
#define M3 5
#define syn 4
unsigned long Moter_time_1 = 3000, Moter_time_2 = 3, Moter_time_3 = 3;
unsigned long U_Moter_time_1 = 3000, U_Moter_time_2 = 2, U_Moter_time_3 = 2;
static struct pt pt1, pt2, pt3;

static int protothread1(struct pt *pt) {
  static unsigned long timestamp = 0;
  static unsigned long timestampe = 0;
  static unsigned long times = 0;
  PT_BEGIN(pt);
  
  while (1) {
   // times+=10000;
//    while (millis() < times) {
      //up
      PT_WAIT_UNTIL(pt, micros() - timestamp > Moter_time_1);
      timestamp = micros();
      digitalWrite(Moter_1, !digitalRead(Moter_1));
      timestampe = micros();
      //down
      PT_WAIT_UNTIL(pt, micros() - timestamp > U_Moter_time_1);
      timestamp = micros();
      digitalWrite(Moter_1, !digitalRead(Moter_1));
      timestampe = micros();
  //  }
   // digitalWrite(Sleep, !digitalRead(Sleep));
   // PT_WAIT_UNTIL(pt, millis() - timestampe > 10000);
   // digitalWrite(Sleep, !digitalRead(Sleep));
  }
  PT_END(pt);
}

static int protothread2(struct pt *pt) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    //up
    PT_WAIT_UNTIL(pt, millis() - timestamp > Moter_time_2);
    timestamp = millis();
    digitalWrite(Moter_2, !digitalRead(Moter_2));
    //down
    PT_WAIT_UNTIL(pt, millis() - timestamp > U_Moter_time_2);
    timestamp = millis();
    digitalWrite(Moter_2, !digitalRead(Moter_2));
  }
  PT_END(pt);
}
static int protothread3(struct pt *pt) {
  static unsigned long timestamp = 0;
  PT_BEGIN(pt);
  while (1) {
    //up
    PT_WAIT_UNTIL(pt, millis() - timestamp > Moter_time_3);
    timestamp = millis();
    digitalWrite(Moter_3, !digitalRead(Moter_3));
    //down
    PT_WAIT_UNTIL(pt, millis() - timestamp > U_Moter_time_3);
    timestamp = millis();
    digitalWrite(Moter_3, !digitalRead(Moter_3));
  }
  PT_END(pt);
}

void setup() {
  pinMode(Moter_1, OUTPUT);
  pinMode(Moter_2, OUTPUT);
  pinMode(Moter_3, OUTPUT);
  pinMode(Sleep, OUTPUT);
  
  pinMode(M1, OUTPUT);
  digitalWrite(M1, !digitalRead(M1));
 pinMode(M2, OUTPUT);
  digitalWrite(M2, digitalRead(M2));
pinMode(M3, OUTPUT);
  digitalWrite(M3, digitalRead(M3));
  
  pinMode(syn, OUTPUT);
  digitalWrite(syn, !digitalRead(syn));
  
  int ledPin = 5;
//analogWrite(ledPin, 200);
  PT_INIT(&pt1);
  PT_INIT(&pt2);
  PT_INIT(&pt3);
}

void loop() {
  
  protothread1(&pt1);
  protothread2(&pt2);
  protothread3(&pt3);
}
