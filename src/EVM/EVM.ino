#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//keypad and lcd intialised
const byte rows = 4;
const byte cols = 4;

byte rowpins[rows] = {9, 8, 7, 6};
byte colpins[cols] = {5, 4, 3, 2};
char keys[rows][cols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

Keypad k = Keypad(makeKeymap(keys), rowpins, colpins, rows, cols);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// input and ouput
const byte btnA = 13;
const byte btnB = 12;
const byte btnC = 11;
const byte resume_button = 10;
const byte mainAdmin = A0;
const byte led = A1;
const byte buzz=A2;

// ID and Admin password
String validID[10] = {"101ABCD", "102AABB", "103BCDA", "104CDAB", "105DDDD", "106ABAB", "107BCBC", "108CDCD", "109DADA", "110ABDC"};
bool voted[10] = {false};
String adminPassword[3] = {"12345", "54321", "98765"};


int vote_count = 0, vote_A = 0, vote_B = 0, vote_C = 0;
int check_Sum = 0;
String enteredID = "";
String enteredPassword = "";
char selectedCandidate = '\0';
int currentVoterIndex = -1;
int adminStep = 0; 
bool resultDisplayed = false;

//states
enum State {
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

State currentState = STATE_VOTING_CLOSED;

//this will be shown when system is on
void showClosed() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VOTING CLOSED");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait");
}

//will show this when admin start voting
void showHome() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER VOTER ID");
}

int findID(String id) {
    for (int i = 0; i < 10; i++) {
        if (validID[i] == id) {
            return i;
        }
    }
    return -1;
}


void checkSum() {
    check_Sum = (vote_A * 7) + (vote_B * 5) + (vote_C * 3);
}

void setup() {
    pinMode(btnA, INPUT_PULLUP);
    pinMode(btnB, INPUT_PULLUP);
    pinMode(btnC, INPUT_PULLUP);
    pinMode(resume_button, INPUT_PULLUP);
    pinMode(mainAdmin, INPUT_PULLUP);  
    pinMode(led, OUTPUT);
    pinMode(buzz, OUTPUT);
    lcd.init();
    lcd.backlight();
    Serial.begin(9600);
    showClosed();}

void loop() {
    if (digitalRead(mainAdmin) == LOW) {
        while (digitalRead(mainAdmin) == LOW);
        adminStep = 0;
        enteredPassword = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ADMIN 1 PASSWORD");
        currentState = STATE_ADMIN_LOGIN;
        return;
    }
    char key = k.getKey();

    switch (currentState) {
        case STATE_VOTING_CLOSED:
            break;

        case STATE_ADMIN_LOGIN: {
            if (key) {
                if (isDigit(key) && enteredPassword.length() < 5) {
                    enteredPassword += key;
                    lcd.setCursor(0, 1);
                    lcd.print(enteredPassword);
                }
                if (key == '#') {
                    if (enteredPassword == adminPassword[adminStep]) {  
                        adminStep++;
                        enteredPassword = "";
                        if (adminStep >= 3) {
                            currentState = STATE_ADMIN_MENU;
                            lcd.clear();
                            lcd.setCursor(0, 0);
                            lcd.print("1.START 2.END");
                            lcd.setCursor(0, 1);
                            lcd.print("3.RESULT");
                            break;
                        } else {
                            lcd.clear();
                            lcd.print("ADMIN");
                            lcd.print(adminStep + 1);
                            lcd.print(" PASSWORD");
                        }
                    } else {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("ACCESS DENIED");
                      	digitalWrite(buzz, HIGH);
                        digitalWrite(led, HIGH);
                        delay(1000);
                        digitalWrite(led, LOW);
                        digitalWrite(buzz, LOW);
                        delay(4000);
                        adminStep = 0;
                        enteredPassword = "";
                        currentState = STATE_VOTING_CLOSED;
                        showClosed();
                    }
                }
            } 
            break;
        }

        case STATE_ADMIN_MENU: {
            if (key == '1') {
                currentState = STATE_IDLE;
                enteredID = "";
                showHome();
            }
            if (key == '2') {
                currentState = STATE_VOTING_CLOSED;
                showClosed();
            }
            if (key == '3') {
                resultDisplayed = false;
                currentState = STATE_SHOW_RESULT;
            }
            break;
        }

        case STATE_IDLE: {
            if (key) {
                if (isalnum(key)) {
                    if (enteredID.length() < 7) {
                        enteredID += key;
                        lcd.setCursor(0, 1);
                        lcd.print("                ");
                        lcd.setCursor(0, 1);
                        lcd.print(enteredID);
                    }
                }
                if (key == '*') {
                  	digitalWrite(led, HIGH);
                  	delay(1000);
                  	digitalWrite(led, LOW);
                    enteredID = "";
                    lcd.setCursor(0, 1);
                    lcd.print("                ");
                }
                if (key == '#') {
                    if (enteredID.length() == 7) {
                        currentVoterIndex = findID(enteredID);
                        if (currentVoterIndex == -1) {
                            currentState = STATE_INVALID_ID;
                        } else if (voted[currentVoterIndex]) {
                            currentState = STATE_ALREADY_VOTED;
                        } else {
                            lcd.clear();
                            lcd.setCursor(0, 0);
                            lcd.print("SELECT");
                            lcd.setCursor(0, 1);
                            lcd.print("CANDIDATE");
                            currentState = STATE_SELECT_CANDIDATE;
                        }
                    } else {
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("ID MUST BE");
                        lcd.setCursor(0, 1);
                        lcd.print("7 CHARS");
                        delay(1500);
                        showHome();
                    }
                    enteredID = "";
                }
            }
            break;
        }

        case STATE_SELECT_CANDIDATE: {
            if (digitalRead(btnA) == LOW) {                
                while (digitalRead(btnA) == LOW);
                if (selectedCandidate != 'A') {    
                    selectedCandidate = 'A';
                    lcd.clear();                   
                }
            }
            if (digitalRead(btnB) == LOW) {               
                while (digitalRead(btnB) == LOW);
                if (selectedCandidate != 'B') {
                    selectedCandidate = 'B';
                    lcd.clear();
                }
            }
            if (digitalRead(btnC) == LOW) {
                while (digitalRead(btnC) == LOW);
                if (selectedCandidate != 'C') {
                    selectedCandidate = 'C';
                    lcd.clear();
                }
            }
            if (selectedCandidate != '\0') {
                lcd.setCursor(0, 0);
                lcd.print("SELECTED:");
                lcd.print(selectedCandidate);
                lcd.setCursor(0, 1);
                lcd.print("# TO CONFIRM");
                if (key == '#') {
                    voted[currentVoterIndex] = true;
                    vote_count++;
                    if (selectedCandidate == 'A') vote_A++;
                    if (selectedCandidate == 'B') vote_B++;
                    if (selectedCandidate == 'C') vote_C++;
                    checkSum(); 
                  	lcd.clear();
                  	lcd.setCursor(0,0);
                  	lcd.print("VOTE CANDIDATE:");
                    lcd.setCursor(7,1);
                    lcd.print(selectedCandidate);
                    
                  	digitalWrite(buzz, HIGH);
                    digitalWrite(led, HIGH);
                    delay(1000);
                    digitalWrite(led, LOW);
                    digitalWrite(buzz, LOW);
                  	delay(4000);
                  	currentVoterIndex = -1;
                    selectedCandidate = '\0';
                    lcd.clear();
                    lcd.print("WAITING ADMIN");
                    currentState = STATE_WAIT_ADMIN;
                }
            }
            break;
        }

        case STATE_INVALID_ID: {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("INVALID ID");
            lcd.setCursor(0, 1);
            lcd.print("GALAT ID");
            digitalWrite(buzz, HIGH);
            digitalWrite(led, HIGH);
            delay(1000);
            digitalWrite(led, LOW);
          	digitalWrite(buzz, LOW);
            enteredID = "";
            showHome();
            currentState = STATE_IDLE;
            break;
        }

        case STATE_ALREADY_VOTED: {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ALREADY VOTED");
            lcd.setCursor(0, 1);
            lcd.print("ACCESS DENIDE");
            digitalWrite(led, HIGH);
          	digitalWrite(buzz, HIGH);
            delay(3000);
            digitalWrite(led, LOW);
          	digitalWrite(buzz, LOW);
            enteredID = "";
            currentVoterIndex = -1;
            showHome();
            currentState = STATE_IDLE;
            break;
        }

        case STATE_WAIT_ADMIN: {
            if (digitalRead(resume_button) == LOW) {
                while (digitalRead(resume_button) == LOW);
                enteredID = "";
                currentVoterIndex = -1;
                selectedCandidate = '\0';
                currentState = STATE_IDLE;
                showHome();
            }
            break;
        }

        case STATE_SHOW_RESULT: {
            if (resultDisplayed == false) {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("TOT:");
                lcd.print(vote_count);
                lcd.print("CHK:");
                lcd.print(check_Sum);
                lcd.setCursor(0, 1);
                lcd.print("A:");
                lcd.print(vote_A);
                lcd.print(" B:");
                lcd.print(vote_B);
                lcd.print("C:");
                lcd.print(vote_C);                            
                resultDisplayed = true;
            }
            if (key == '#') {
                resultDisplayed = false;
                currentState = STATE_ADMIN_MENU;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("1.START 2.END");
                lcd.setCursor(0, 1);
                lcd.print("3.RESULT");
            }
            break;
        }
    }
}