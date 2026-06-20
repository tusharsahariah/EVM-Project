
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const byte rows=4, cols=4;
byte rowPins[rows]={9,8,7,6};
byte colPins[cols]={5,4,3,2};

char keys[rows][cols] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};

Keypad k = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);
LiquidCrystal_I2C lcd(0x27,16,2);

int btnA=13, btnB=12, btnC=11;
int resume_button=10;
int mainAdminBtn=A0;
int ledE=A1;

String validIDs[10]={"101ABCD","102AABB","103BCDA","104CDAB","105DDDD","106ABAB","107BCBC","108CDCD","109DADA","110ABDC"};
bool voted[10]={false};

String adminPasswords[3]={"12345","54321","98765"};

int vote_count=0,vote_A=0,vote_B=0,vote_C=0;
long check_Sum=0;

String enteredID="", enteredPassword="";
char selectedCandidate='\0';
int currentVoterIndex=-1;
int adminStep=0;

enum State{
STATE_VOTING_CLOSED,
STATE_ADMIN_LOGIN,
STATE_ADMIN_MENU,
STATE_IDLE,
STATE_SELECT_CANDIDATE,
STATE_INVALID_ID,
STATE_ALREADY_VOTED,
STATE_WAIT_ADMIN,
STATE_SHOW_RESULT
};

State currentState=STATE_VOTING_CLOSED;

int findID(String id){
 for(int i=0;i<10;i++) if(validIDs[i]==id) return i;
 return -1;
}

void showClosed(){
 lcd.clear();
 lcd.setCursor(0,0); lcd.print("VOTING CLOSED");
 lcd.setCursor(0,1); lcd.print("PRESS ADMIN");
}

void setup(){
 pinMode(btnA,INPUT_PULLUP);
 pinMode(btnB,INPUT_PULLUP);
 pinMode(btnC,INPUT_PULLUP);
 pinMode(resume_button,INPUT_PULLUP);
 pinMode(mainAdminBtn,INPUT_PULLUP);
 pinMode(ledE,OUTPUT);

 lcd.init(); lcd.backlight();
 showClosed();
}

void loop(){

 if(digitalRead(mainAdminBtn)==LOW){
   while(digitalRead(mainAdminBtn)==LOW);
   adminStep=0;
   enteredPassword="";
   currentState=STATE_ADMIN_LOGIN;
   lcd.clear();
   lcd.print("ADMIN1 PASS");
 }

 char key=k.getKey();

 switch(currentState){

 case STATE_VOTING_CLOSED:
   break;

 case STATE_ADMIN_LOGIN:
   if(key){
      if(isDigit(key) && enteredPassword.length()<5){
        enteredPassword+=key;
        lcd.setCursor(0,1); lcd.print(enteredPassword);
      }
      if(key=='#'){
        if(enteredPassword==adminPasswords[adminStep]){
          adminStep++;
          enteredPassword="";
          if(adminStep>=3){
             currentState=STATE_ADMIN_MENU;
             lcd.clear();
             lcd.print("1S 2E 3R");
          }else{
             lcd.clear();
             lcd.print("ADMIN");
             lcd.print(adminStep+1);
             lcd.print(" PASS");
          }
        }else{
          currentState=STATE_VOTING_CLOSED;
          showClosed();
        }
      }
   }
   break;

 case STATE_ADMIN_MENU:
   if(key=='1'){
      currentState=STATE_IDLE;
      lcd.clear(); lcd.print("ENTER YOUR ID");
   }
   if(key=='2'){
      currentState=STATE_VOTING_CLOSED;
      showClosed();
   }
   if(key=='3'){
      currentState=STATE_SHOW_RESULT;
      lcd.clear();
      lcd.print("TOT:");
      lcd.print(vote_count);
      lcd.setCursor(0,1);
      lcd.print("A:");
      lcd.print(vote_A);
      lcd.print(" B:");
      lcd.print(vote_B);
   }
   break;

 case STATE_SHOW_RESULT:
   if(key=='#'){
      currentState=STATE_ADMIN_MENU;
      lcd.clear(); lcd.print("1S 2E 3R");
   }
   break;

 case STATE_IDLE:
   if(key && isalnum(key)){
      if(enteredID.length()<7){
        enteredID+=key;
        lcd.setCursor(0,1); lcd.print(enteredID);
      }
   }
   if(key=='#'){
      currentVoterIndex=findID(enteredID);
      if(currentVoterIndex<0){
        digitalWrite(ledE,HIGH); delay(1000); digitalWrite(ledE,LOW);
      }else if(voted[currentVoterIndex]){
        digitalWrite(ledE,HIGH); delay(1000); digitalWrite(ledE,LOW);
      }else{
        currentState=STATE_SELECT_CANDIDATE;
        lcd.clear(); lcd.print("SELECT CAND");
      }
      enteredID="";
   }
   break;

 case STATE_SELECT_CANDIDATE:
   if(digitalRead(btnA)==LOW) selectedCandidate='A';
   if(digitalRead(btnB)==LOW) selectedCandidate='B';
   if(digitalRead(btnC)==LOW) selectedCandidate='C';

   if(selectedCandidate!='\0'){
      lcd.setCursor(0,1); lcd.print(selectedCandidate);
      if(key=='#'){
        voted[currentVoterIndex]=true;
        vote_count++;
        if(selectedCandidate=='A') vote_A++;
        if(selectedCandidate=='B') vote_B++;
        if(selectedCandidate=='C') vote_C++;
        check_Sum=(vote_A*7)+(vote_B*5)+(vote_C*3);

        selectedCandidate='\0';
        currentState=STATE_WAIT_ADMIN;
        lcd.clear(); lcd.print("VOTE CASTED");
        delay(1000);
        lcd.clear(); lcd.print("WAIT ADMIN");
      }
   }
   break;

 case STATE_WAIT_ADMIN:
   if(digitalRead(resume_button)==LOW){
      while(digitalRead(resume_button)==LOW);
      currentState=STATE_IDLE;
      lcd.clear(); lcd.print("ENTER YOUR ID");
   }
   break;

 default: break;
 }
}
