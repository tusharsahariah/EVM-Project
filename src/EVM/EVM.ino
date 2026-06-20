#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//===================== KEYPAD =====================

const byte rows = 4;
const byte cols = 4;

byte rowPins[rows] = {9, 8, 7, 6};
byte colPins[cols] = {5, 4, 3, 2};

char keys[rows][cols] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

Keypad k = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

//===================== LCD =====================

LiquidCrystal_I2C lcd(0x27, 16, 2);

//===================== PINS =====================

int A = 13;
int B = 12;
int C = 11;
int resume_button=10;

int ledA = A3;
int ledB = A2;
int ledC = A1;
int ledE = A0;

//===================== VOTER DATABASE =====================

String validIDs[10] = {
  "101ABCD",
  "102AABB",
  "103BCDA",
  "104CDAB",
  "105DDDD",
  "106ABAB",
  "107BCBC",
  "108CDCD",
  "109DADA",
  "110ABDC"
};

bool voted[10] = {false};

//===================== VARIABLES =====================

String enteredID = "";
char selectedCandidate = '\0';
int currentVoterIndex = -1;

//===================== STATES =====================

enum State {
  STATE_IDLE,
  STATE_SELECT_CANDIDATE,
  STATE_INVALID_ID,
  STATE_ALREADY_VOTED,
  STATE_VOTE_SUCCESS,
  STATE_VOTING_CLOSED
};
State currentState = STATE_IDLE;

//===================== FUNCTIONS =====================

void showHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER YOUR ID");
}

int findID(String id) {
  for (int i = 0; i < 10; i++) {
    if (validIDs[i] == id) {
      return i;
    }
  }
  return -1;
}

void showInvalidID() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("INVALID ID");
  digitalWrite(ledE, HIGH);

  delay(3000);
  digitalWrite(ledE, LOW);

  enteredID = "";
  showHome();
  currentState = STATE_IDLE;
}

void showAlreadyVoted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALREADY VOTED");
  digitalWrite(ledE, HIGH);
  delay(3000);
  digitalWrite(ledE, LOW);

  enteredID = "";
  showHome();
  currentState = STATE_IDLE;
}

void showVoteSuccess()
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("VOTE CASTED");

  delay(3000);

  enteredID = "";
  selectedCandidate = '\0';
  currentVoterIndex = -1;

  currentState = STATE_VOTING_CLOSED;
}
void showVotingClosed()
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("VOTING PAUSED");

  while(digitalRead(resume_button) == HIGH)
  {
    // Wait for admin
  }

  while(digitalRead(resume_button) == LOW);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("VOTING RESUMED");

  delay(1000);

  showHome();

  currentState = STATE_IDLE;
}
void handleIDEntry() {

  char key = k.getKey();

  if (key) {

    if (isalnum(key)) {

      if (enteredID.length() < 7) {

        enteredID += key;
        lcd.setCursor (0,1);
        lcd.print("           ");

        

        lcd.setCursor(0, 1);
        lcd.print(enteredID);
      }
    }

    if (key == '*') {

      enteredID = "";

      lcd.setCursor(0, 1);
      lcd.print("                ");
    }

    if (key == '#') {

      if (enteredID.length() == 7) {

        currentVoterIndex = findID(enteredID);

        if (currentVoterIndex == -1) {

          currentState = STATE_INVALID_ID;
        }
        else if (voted[currentVoterIndex]) {

          currentState = STATE_ALREADY_VOTED;
        }
        else {

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("SELECT");

          lcd.setCursor(0, 1);
          lcd.print("CANDIDATE");

          currentState = STATE_SELECT_CANDIDATE;
        }
      }
    }
  }
}

void handleCandidateSelection() {

  if (digitalRead(A) == LOW) {

    selectedCandidate = 'A';

    while (digitalRead(A) == LOW);

    digitalWrite(ledA, HIGH);

    selectCandidate();
  }

  if (digitalRead(B) == LOW) {

    selectedCandidate = 'B';

    while (digitalRead(B) == LOW);

    digitalWrite(ledB, HIGH);

    selectCandidate();
  }

  if (digitalRead(C) == LOW) {

    selectedCandidate = 'C';

    while (digitalRead(C) == LOW);

    digitalWrite(ledC, HIGH);

    selectCandidate();
  }
}

void selectCandidate()
{
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("CANDIDATE ");
  lcd.print(selectedCandidate);

  lcd.setCursor(0,1);
  lcd.print("Enter to Confirm");

  unsigned long startTime = millis();

  while(millis() - startTime < 10000)
  {
    char key = k.getKey();

    if(key == '#')
    {
      voted[currentVoterIndex] = true;

      digitalWrite(ledA, LOW);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, LOW);

      digitalWrite(ledE, HIGH);

      lcd.clear();

      lcd.setCursor(0,0);
      lcd.print("VOTED TO ");
      lcd.print(selectedCandidate);

      delay(3000);

      digitalWrite(ledE, LOW);

      currentState = STATE_VOTE_SUCCESS;

      return;
    }
  }

  digitalWrite(ledA, LOW);
  digitalWrite(ledB, LOW);
  digitalWrite(ledC, LOW);
  digitalWrite(ledE, LOW);

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("TIME OUT");

  delay(3000);

  selectedCandidate = '\0';

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("SELECT");

  lcd.setCursor(0,1);
  lcd.print("CANDIDATE");
}
//===================== SETUP =====================

void setup() {

  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  pinMode(C, INPUT_PULLUP);
  pinMode(resume_button, INPUT_PULLUP);

  pinMode(ledA, OUTPUT);
  pinMode(ledB, OUTPUT);
  pinMode(ledC, OUTPUT);
  pinMode(ledE, OUTPUT);

  lcd.init();
  lcd.backlight();

  Serial.begin(9600);

  showHome();
}

//===================== LOOP =====================
void loop() {

  switch (currentState) {

    case STATE_IDLE:
      handleIDEntry();
      break;

    case STATE_SELECT_CANDIDATE:
      handleCandidateSelection();
      break;

    case STATE_INVALID_ID:
      showInvalidID();
      break;

    case STATE_ALREADY_VOTED:
      showAlreadyVoted();
      break;

    case STATE_VOTE_SUCCESS:
      showVoteSuccess();
      break;
    case STATE_VOTING_CLOSED:
      showVotingClosed();
      break;
  }
}