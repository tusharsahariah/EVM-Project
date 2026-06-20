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
int enter = 10;

int ledA = A3;
int ledB = A2;
int ledC = A1;
int ledE = A0;

//===================== VOTER DATABASE =====================

String validIDs[10] = {
  "101ABC",
  "102DEF",
  "103GHI",
  "104JKL",
  "105MNO",
  "106PQR",
  "107STU",
  "108VWX",
  "109YZA",
  "110BCD"
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
  STATE_VOTE_SUCCESS
};

State currentState = STATE_IDLE;

//===================== FUNCTIONS =====================

void showHome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ENTER YOUR ID");

  lcd.setCursor(0, 1);
  lcd.print("******");
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

  delay(3000);

  enteredID = "";
  showHome();
  currentState = STATE_IDLE;
}

void showAlreadyVoted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALREADY VOTED");

  delay(3000);

  enteredID = "";
  showHome();
  currentState = STATE_IDLE;
}

void showVoteSuccess() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("VOTE CASTED");

  delay(3000);

  enteredID = "";
  selectedCandidate = '\0';
  currentVoterIndex = -1;

  showHome();

  currentState = STATE_IDLE;
}

void handleIDEntry() {

  char key = k.getKey();

  if (key) {

    if (isalnum(key)) {

      if (enteredID.length() < 6) {

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

      if (enteredID.length() == 6) {

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

void selectCandidate() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CANDIDATE ");
  lcd.print(selectedCandidate);

  lcd.setCursor(0, 1);
  lcd.print("Press Enter");

  unsigned long startTime = millis();

  while (millis() - startTime < 10000) {

    if (digitalRead(enter) == LOW) {

      while (digitalRead(enter) == LOW);

      voted[currentVoterIndex] = true;

      digitalWrite(ledA, LOW);
      digitalWrite(ledB, LOW);
      digitalWrite(ledC, LOW);
      digitalWrite(ledE, HIGH);

      lcd.clear();

      lcd.setCursor(0, 0);
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

  lcd.setCursor(0, 0);
  lcd.print("TIME OUT");

  delay(3000);

  selectedCandidate = '\0';

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SELECT");

  lcd.setCursor(0, 1);
  lcd.print("CANDIDATE");
}

//===================== SETUP =====================

void setup() {

  pinMode(A, INPUT_PULLUP);
  pinMode(B, INPUT_PULLUP);
  pinMode(C, INPUT_PULLUP);
  pinMode(enter, INPUT_PULLUP);

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

    case IDLE:
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
  }
}