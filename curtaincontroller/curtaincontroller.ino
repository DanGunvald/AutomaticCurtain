#include <Button.h>
#include <EEPROM.h>

//#define DEBUG 1

volatile long curpos = 0;
volatile long curservopos = 0;
volatile bool gotopos = false;
volatile bool sleeping = true;
volatile long wantedpos = 0;
volatile long wantedservopos = 0;
static char serbuffer[64];
static int serbufferpos = 0;
long pendingwantedpos = -1;
long pendingwantedservopos = -1;
long startupdelay = 200;
int direction = 0;
volatile bool servoPulse = false;
volatile int ss =  0;
long ldummy;
int stepsprsspeed = 10;
long actualstepsthisrun = 0;
long lastsetocr = 0;
long delta = 0;


#define DIR 2
#define STEP 3
#define SLEEP 4
//ative low
#define RESET 5
//active low
#define MS3 6
#define MS2 7
#define MS1 8
#define ENABLE 9
//active low
#define SRVENABLE 10
#define SERVO 11
#define SERVOTOTAL 40000
#define SERVOMIN 1000
#define SERVOMAX 5000
#define SERVOSTEPS 180
#define SERVOSTART 3000

volatile int servoIntPos = SERVOTOTAL -SERVOSTART;
volatile int servoIntPos1 = SERVOSTART;
volatile int maxspeed = 255;
int servopos0 = 1000;
int servopos180 = 5000;
int servostep = (SERVOMAX - SERVOMIN) / SERVOSTEPS;


struct settings {
  int directionstepper;
  int ms1;
  int ms2;
  int ms3;
  long steps;
  long skipstepsonstartfromzero;
  long servoup;
  long servodown;
  long servopredelay;
  long servopostdelay;
  int autoservo;
  long servomovedelay;
  int maxspeedup;
  int maxspeeddown;
};

settings sets;

void loadDefaults() {
    sets.steps = 65000;
    sets.skipstepsonstartfromzero = 200;
    sets.directionstepper = 1;
    sets.ms1 = 1;
    sets.ms2 = 1;
    sets.ms3 = 1;
    sets.servoup = 120;
    sets.servodown = 60;
    sets.servopredelay=200;
    sets.servopostdelay=700;
    sets.autoservo=1;
    sets.servomovedelay=2;
    sets.maxspeedup = 40;
    sets.maxspeeddown = 35;
}

void ReedEeprom() {
  if (EEPROM[0] == 255 && EEPROM[1] == 255) {
    loadDefaults();
    //assume uninitialzed
    StoreEeprom();
    Serial.println("initializing eeprom");
  } else {
    EEPROM.get(0, sets);
  }
}

void StoreEeprom() {
  EEPROM.put(0, sets);
  return;
}

ISR(TIMER2_COMPA_vect){
 if (direction != 0 && curpos != wantedpos && gotopos) {
    curpos += direction;
    actualstepsthisrun ++;
    digitalWrite(STEP, HIGH);
    digitalWrite(STEP, LOW);
    delta = abs(curpos - wantedpos);
    if (delta <= stepsprsspeed*(255-maxspeed)) {
      //slowdown
      lastsetocr = 255 - (delta / stepsprsspeed);
      if (lastsetocr > 255) {
        lastsetocr =255;
      }
      if (lastsetocr < maxspeed) {
        lastsetocr = maxspeed;
      }
      OCR2A = lastsetocr;
    } else {
      if (lastsetocr != maxspeed) {
        if ((actualstepsthisrun / stepsprsspeed) > 255- maxspeed) {
          
          lastsetocr = maxspeed;
           if (lastsetocr > 255) {
            lastsetocr =255;
          }
          OCR2A = lastsetocr;
        } else {
          lastsetocr = 255 - (actualstepsthisrun / stepsprsspeed);
           if (lastsetocr > 255) {
            lastsetocr =255;
          }
          OCR2A = lastsetocr;
        }
      }
    }
  }
}


void DebugPrint() {
return;
  Serial.print("direction:");
  Serial.println(direction);
    Serial.print("curpos:");
  Serial.println(curpos);
    Serial.print("wantedpos:");
  Serial.println(wantedpos);
}
void gotoPos(long dummy, bool move = false) {
  if (dummy >= 0 && dummy <= sets.steps) {
    if (gotopos) {
#ifdef DEBUG
    Serial.println("allready going");
#endif
      //active
      pendingwantedpos = dummy;
    } else {
#ifdef DEBUG
    Serial.print("Going to");
    Serial.println(dummy);
#endif
      if (curpos == 0 && dummy > sets.skipstepsonstartfromzero) {
        curpos = sets.skipstepsonstartfromzero;
      } 
      wantedpos = dummy;

      if (wantedpos != curpos) {
        actualstepsthisrun = 0;
        //wantedpos = dummy;
        if (wantedpos > curpos) {
          //going down
          maxspeed = sets.maxspeeddown;
          SetDirection(0);
          direction = 1;
        } else {
          maxspeed = sets.maxspeedup;
          SetDirection(1);
          direction = -1;
          //going up
        }
        if (sleeping) {
          wakeUp();
          sleeping = false;
        }
        gotopos = true;
      } else {
#ifdef DEBUG
        Serial.println("alleready here");
#endif
      }
    }
  } else {
  //  Serial.println("not going");
  }
}

long changeservoposcountdown = -1;
int tmpS = 0;
bool servoSleep = true;
long servoStarted = -1;
ISR(TIMER1_COMPA_vect){
  if (servoPulse) {
    servoPulse = false;
    OCR1A = servoIntPos1;
    digitalWrite(SERVO, HIGH);
  } else {
    servoPulse = true;
    OCR1A = servoIntPos;
    digitalWrite(SERVO, LOW);
    if (curservopos != wantedservopos) {
      if (servoSleep) {
        digitalWrite(SRVENABLE, HIGH);
        servoStarted = millis();
        servoSleep = false;
      }  else {
        changeservoposcountdown --;
        if (changeservoposcountdown <= 0) {
          if (curservopos < wantedservopos) {
            curservopos ++;
          } else if (curservopos > wantedservopos) {
            curservopos --;
          }
          tmpS = (curservopos * servostep);
          servoIntPos = (SERVOTOTAL - SERVOMIN) - (tmpS);
          servoIntPos1 = SERVOMIN + tmpS;
          changeservoposcountdown = sets.servomovedelay;
        }
      }
    }
  }
}

void setServoPos(long pos) {
  wantedservopos = pos;
  changeservoposcountdown = sets.servomovedelay;
//  printstatus(2);
}

/*
void ActualSetServoPos(long pos) {
  while (!servoPulse) ;
  while (servoPulse) ;
  int s = (pos* servostep);
  //Serial.println(s);
  //Serial.println("cli");
  cli();
  
  servoIntPos = (SERVOTOTAL - SERVOMIN) - (s);
  servoIntPos1 = SERVOMIN + s;
//  volatile int servoIntPos = 40000 -3000;
//volatile int servoIntPos1 = 3000;
//int servopos0 = 2000;
//int servopos180 = 4000;
  sei();
  //Serial.println("sei");
}
*/


void setupTimerInterruptForStepper() {
  cli();
  TCCR2A = 0;// set entire TCCR0A register to 0
  TCCR2B = 0;// same for TCCR0B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 2khz increments
  OCR2A = 255;// = (16*10^6) / (2000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR2B |=  (1 << CS22) ;   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);
  sei();
}

void setupTimerInterruptForServo() {
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 40000;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 8 prescaler
  TCCR1B |=   (1 << CS11);  ;  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void SetDirection(int wanteddirection) {
  if (sets.directionstepper == 1) {
    if (wanteddirection == 1) {
      //guino down
      digitalWrite(DIR, HIGH);
    } else {
      digitalWrite(DIR, LOW);
    }
  } else {
    if (wanteddirection == 1) {
      //guino down
      digitalWrite(DIR, LOW);
    } else {
      digitalWrite(DIR, HIGH);
    }
  }
}

void setup() {
  Serial.begin(115200);
  ReedEeprom();
  pinMode(SERVO, OUTPUT);
  pinMode(SERVO, OUTPUT);
  pinMode(SRVENABLE, OUTPUT);
  digitalWrite(SRVENABLE,LOW);
  digitalWrite(SERVO, LOW);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  digitalWrite(12, LOW);
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(SLEEP, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  digitalWrite(DIR, LOW);
  digitalWrite(STEP, LOW);
  digitalWrite(SLEEP, LOW);
  digitalWrite(RESET, LOW);
  if (sets.ms1 == 0) {
    digitalWrite(MS1, LOW);
  } else {
    digitalWrite(MS1, HIGH);
  }
  if (sets.ms2 == 0) {
    digitalWrite(MS2, LOW);
  } else {
    digitalWrite(MS2, HIGH);
  }
  if (sets.ms3 == 0) {
    digitalWrite(MS3, LOW);
  } else {
    digitalWrite(MS3, HIGH);
  }
  digitalWrite(ENABLE, LOW);
  gotoSleep();
  setupTimerInterruptForServo();
  setupTimerInterruptForStepper();
  printstatus(3);
}

void wakeUp() {
  digitalWrite(ENABLE, LOW);
  digitalWrite(SLEEP, HIGH);
  digitalWrite(RESET, HIGH);
  digitalWrite(13, HIGH);
  delay(100);
#ifdef DEBUG
  Serial.println("wakeup");
#endif
}

void printstatus(int i) {
  if (i & 1) {
    Serial.print("curpos:");
    Serial.println(curpos);
    
  }
  if (i & 2) {  
     
     delay(120);
    Serial.print("curservopos:");
    Serial.println(curservopos);
  }
 
  if (i & 4) {   
        delay(120);
  
    Serial.print("wantedservopos:");
    Serial.println(wantedservopos);
  }
  if (i & 8) {
    delay(120);
    Serial.print("wantedpos:");
    Serial.println(wantedpos);
  }
}


void printvalues() {
  Serial.print("curpos:");
  Serial.println(curpos);
  delay(120);
  Serial.print("steps:");
  Serial.println(sets.steps);
  delay(120);
  Serial.print("skipstepsonstartfromzero:");
  Serial.println(sets.skipstepsonstartfromzero);
  delay(120);
  Serial.print("directionstepper:");
  Serial.println(sets.directionstepper);
  delay(120);
  Serial.print("ms1:");
  Serial.println(digitalRead(MS1));
  delay(120);
  Serial.print("ms2:");
  Serial.println(digitalRead(MS2));
  delay(120);
  Serial.print("ms3:");
  Serial.println(digitalRead(MS3));
  delay(120);
  Serial.print("servouppos:");
  Serial.println(sets.servoup);
  delay(120);
  Serial.print("servodownpos:");
  Serial.println(sets.servodown);
  delay(120);
  Serial.print("servopredelay:");
  Serial.println(sets.servopredelay);
  delay(120);
  Serial.print("servopostdelay:");
  Serial.println(sets.servopostdelay);
  delay(120);
  Serial.print("autoservo:");
  Serial.println(sets.autoservo);
  delay(120);
  Serial.print("servomovedelay:");
  Serial.println(sets.servomovedelay);
  delay(120);
  Serial.print("maxspeedup:");
  Serial.println(sets.maxspeedup);
  delay(120);
  Serial.print("maxspeeddown:");
  Serial.println(sets.maxspeeddown);
  
}

void gotoSleep() {
  lastsetocr = 255;
  OCR2A = 255;
  delay(100);
  //digitalWrite(ENABLE, HIGH);
  digitalWrite(SLEEP, LOW);
  digitalWrite(RESET, HIGH);
  digitalWrite(13, LOW);
  //printstatus(1);
#ifdef DEBUG
  Serial.println("Sleep");
#endif
}





void settings(char *strbuf) {
    String str = strbuf;
    String key = str.substring(0,str.indexOf(':'));
    String value = str.substring(str.indexOf(':') + 1);
    if (key.length() > 0 && value.length() > 0 ) {
      long val = value.toInt();
      if (key == "ms1") {
        if (val == 1 ) {
          digitalWrite(MS1, HIGH);
          sets.ms1 = 1;
        } else {
          digitalWrite(MS1, LOW);
          sets.ms1 = 0;
        }
      }
      if (key == "ms2") {
        if (val == 1 ) {
          digitalWrite(MS2, HIGH);
          sets.ms2 = 1;
        } else {
          digitalWrite(MS2, LOW);
          sets.ms2 = 0;
        }
      }
      if (key == "ms3") {
        if (val == 1 ) {
          digitalWrite(MS3, HIGH);
          sets.ms3 = 1;
        } else {
          digitalWrite(MS3, LOW);
          sets.ms3 = 0;
        }
      }
      if (key =="servomovedelay") {
        if (val >= 0 ) {
          sets.servomovedelay = val;
        }
      }
      if (key == "directionstepper") {
        if (val == 1) {
          sets.directionstepper = 1;
        } else {
          sets.directionstepper = 0;
        }
      }
      if (key == "steps") {
        if (val > 1) {
          sets.steps = val;
        }
      }  
      if (key == "skipstepsonstartfromzero") {
        if (val >= 0) {
          sets.skipstepsonstartfromzero = val;
        }
      }
      if (key ==  "curpos") {
        if (val > -1) {
          curpos = val;
        }
      }
      if (key ==  "maxspeedup") {
        if (val > -1) {
          sets.maxspeedup = val;
        }
      }
      if (key ==  "maxspeeddown") {
        if (val > -1) {
          sets.maxspeeddown = val;
        }
      }
      
      if (key == "servopostdelay") {
        if (val >= 0) {
          sets.servopostdelay = val;
        }
      }
      if (key == "servopredelay") {
        if (val >= 0) {
          sets.servopredelay = val;
        }
      }
      if (key ==  "autoservo") {
        if (val == 0) {
          sets.autoservo = 0;
        } else {
          sets.autoservo = 1;
        }
      }
      if (key ==  "servouppos") {
        if (val >= 0) {
          sets.servoup = val;
        }
      }
      if (key == "servodownpos") {
        if (val >= 0) {
        sets.servodown = val;
      }
    }
  }
}


void commands() {
  if (strncmp(serbuffer, "sethome", 7) == 0) {
    curpos = 0;
  }
      if (strncmp("store", serbuffer, 5) == 0) {
        StoreEeprom();
      }
      if (strncmp("status", serbuffer, 6) == 0) {
        printstatus(3);
      }
      if (strncmp("defaults", serbuffer, 8) == 0) {
        printvalues();
        loadDefaults();
        StoreEeprom(); 
        printvalues();   
      }
      
      if (strncmp("values", serbuffer, 6) == 0) {
        printvalues();
      }
      if (strncmp(serbuffer, "servodown", 9) == 0) {
        setServoPos(sets.servodown);
      }
      if (strncmp(serbuffer, "servoup", 7) == 0) {
        setServoPos(sets.servoup);
      }
      if (strncmp(serbuffer, "down", 4) == 0) {
        if (sets.autoservo) {
          setServoPos(sets.servodown);
        }
        gotoPos(sets.steps);
      }
      if (strncmp(serbuffer, "up", 2) == 0) {
        gotoPos(0,true);
        if (sets.autoservo) {
         setServoPos(sets.servoup);
        }
      }
      if (strncmp(serbuffer, "gotopos:", 8) == 0) {
        String str = serbuffer + 8;
        long dummy = str.toInt();
        gotoPos(dummy);
      }
      if (strncmp(serbuffer, "gotoservopos:", 13) == 0) {
        String str = serbuffer + 13;
        long dummy = str.toInt();
        if (dummy >= 0) {
          setServoPos(dummy);
        }
      }
}

long lastStatus = 0;

void loop() {
  DebugPrint();
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == 10 || c == 13) {
      serbuffer[serbufferpos] = 0;
      serbufferpos = 0;
      if (strncmp(serbuffer, "set ", 4) == 0) {
        settings(serbuffer + 4);  
        //set
      } else {
        commands();
      }
    } else {
      serbuffer[serbufferpos++] = c;
    }
    if (serbufferpos > 60) {
      serbufferpos = 0;
    }
  }
  if (gotopos) {
    #ifdef DEBUG
      if (DEBUG > 2) {
        Serial.println(lastsetocr);
      }
    #endif
  }

#ifdef DEBUG
  if (curpos != wantedpos || curservopos != wantedservopos) {
    printstatus(15);
    Serial.println(millis());
        lastStatus = millis();

  }
#endif
  if (lastStatus + 60000 < millis()) {
    printstatus(3);
    lastStatus = millis();
  }
  if (gotopos && wantedpos==curpos) {
    if (pendingwantedpos != -1) {
      gotopos = false;
      gotoPos(pendingwantedpos);
//      wantedpos = pendingwantedpos;
      pendingwantedpos = -1;
      printstatus(3);
    } else {
      gotoSleep();
      gotopos = false;
      sleeping = true;
      printstatus(3);
    }
  }
   if (changeservoposcountdown != -1 && curservopos == wantedservopos) {
    changeservoposcountdown = -1;
    printstatus(3);
    digitalWrite(SRVENABLE, LOW);
    servoSleep = true;
   } 
 }
