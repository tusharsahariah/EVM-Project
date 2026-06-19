#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

int A=11;
int B=10;
int C=9;
int enter=8;

char selectedCandidate='\0';
void showHome(){

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SELECT");

    lcd.setCursor(0,1);
    lcd.print("CANDIDATE");
}
void setup(){

    pinMode(A,INPUT_PULLUP);
    pinMode(B,INPUT_PULLUP);
    pinMode(C,INPUT_PULLUP);
    pinMode(enter,INPUT_PULLUP);

    lcd.init();
    lcd.backlight();

    Serial.begin(9600);

    showHome();
}
void loop(){

    if(digitalRead(A)==LOW){

        selectedCandidate='A';

        while(digitalRead(A)==LOW); //wait release

        selectCandidate();
    }

    if(digitalRead(B)==LOW){

        selectedCandidate='B';

        while(digitalRead(B)==LOW);

        selectCandidate();
    }

    if(digitalRead(C)==LOW){

        selectedCandidate='C';

        while(digitalRead(C)==LOW);

        selectCandidate();
    }
}
void selectCandidate(){

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("CANDIDATE ");
    lcd.print(selectedCandidate);

    lcd.setCursor(0,1);
    lcd.print("Press Enter");

    unsigned long startTime=millis();

    while(millis()-startTime<10000){

        if(digitalRead(enter)==LOW){

            while(digitalRead(enter)==LOW);

            lcd.clear();

            lcd.setCursor(0,0);
            lcd.print("VOTED TO ");
            lcd.print(selectedCandidate);

            delay(5000);

            selectedCandidate='\0'; //reset

            showHome();

            return;
        }
    }

    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("TIME OUT");

    delay(1500);

    selectedCandidate='\0'; //reset

    showHome();
}
