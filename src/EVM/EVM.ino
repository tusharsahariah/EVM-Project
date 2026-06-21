#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>


#define EEPROM_MAGIC_ADDR       0   
#define EEPROM_VOTE_A_ADDR      2   
#define EEPROM_VOTE_B_ADDR      4   
#define EEPROM_VOTE_C_ADDR      6   
#define EEPROM_VOTE_TOT_ADDR    8   
#define EEPROM_VOTED_ADDR       10  
#define EEPROM_OPEN_ADDR        20  
#define EEPROM_LOCKED_ADDR      21  
#define EEPROM_CHECKSUM_ADDR    22 
#define MAGIC_BYTE_0  0x45   // 'E'
#define MAGIC_BYTE_1  0x56   // 'V'
#define NUM_VOTERS    10

//  KEYPAD & LCD 
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


const byte btnA          = 13;
const byte btnB          = 12;
const byte btnC          = 11;
const byte resume_button = 10;
const byte mainAdmin     = A0;
const byte led           = A1;
const byte buzz          = A2;
const byte confirm       = A3;


String validID[NUM_VOTERS] = {
    "101ABCD", "102AABB", "103BCDA", "104CDAB", "105DDDD",
    "106ABAB", "107BCBC", "108CDCD", "109DADA", "110ABDC"
};
bool voted[NUM_VOTERS] = {false};
String adminPassword = "12345";


int vote_count = 0, vote_A = 0, vote_B = 0, vote_C = 0;
uint32_t storedChecksum  = 0;   
uint32_t currentChecksum = 0;   

String enteredID       = "";
String enteredPassword = "";
char   selectedCandidate = '\0';
int    currentVoterIndex = -1;
bool   resultDisplayed   = false;
bool   electionOpen      = false;  
bool   dataCorrupted     = false;  
bool   systemLocked      = false;  
int    adminFailCount    = 0;      

//  STATE MACHINE 
enum State {
    STATE_VOTING_CLOSED,
    STATE_ADMIN_LOGIN,
    STATE_ADMIN_MENU,
    STATE_IDLE,
    STATE_SELECT_CANDIDATE,
    STATE_INVALID_ID,
    STATE_ALREADY_VOTED,
    STATE_WAIT_ADMIN,
    STATE_SHOW_RESULT,
    STATE_RESET_CONFIRM,   
    STATE_DATA_CORRUPTED, 
    STATE_SYSTEM_LOCKED    
};

State currentState = STATE_VOTING_CLOSED;


uint32_t calculateChecksum() {
    uint32_t cs = 0xDEADBEEF;

    cs += (uint32_t)vote_A   * 7919UL;
    cs += (uint32_t)vote_B   * 6271UL;
    cs += (uint32_t)vote_C   * 4987UL;
    cs += (uint32_t)vote_count * 3541UL;

    for (int i = 0; i < NUM_VOTERS; i++) {
        if (voted[i]) cs ^= (1UL << i);
    }

   
    cs += (uint32_t)electionOpen * 1234567UL;

    return cs;
}


void saveChecksum() {
    currentChecksum = calculateChecksum();
    storedChecksum  = currentChecksum;
    EEPROM.put(EEPROM_CHECKSUM_ADDR, currentChecksum);
}


void saveAllData() {
    EEPROM.put(EEPROM_VOTE_A_ADDR,   vote_A);
    EEPROM.put(EEPROM_VOTE_B_ADDR,   vote_B);
    EEPROM.put(EEPROM_VOTE_C_ADDR,   vote_C);
    EEPROM.put(EEPROM_VOTE_TOT_ADDR, vote_count);

    for (int i = 0; i < NUM_VOTERS; i++) {
        EEPROM.update(EEPROM_VOTED_ADDR + i, (byte)voted[i]);
    }

    EEPROM.update(EEPROM_OPEN_ADDR,   (byte)electionOpen);
    EEPROM.update(EEPROM_LOCKED_ADDR, (byte)systemLocked);

    saveChecksum();  
}


bool loadAllData() {
    EEPROM.get(EEPROM_VOTE_A_ADDR,   vote_A);
    EEPROM.get(EEPROM_VOTE_B_ADDR,   vote_B);
    EEPROM.get(EEPROM_VOTE_C_ADDR,   vote_C);
    EEPROM.get(EEPROM_VOTE_TOT_ADDR, vote_count);

    for (int i = 0; i < NUM_VOTERS; i++) {
        voted[i] = (bool)EEPROM.read(EEPROM_VOTED_ADDR + i);
    }

    electionOpen  = (bool)EEPROM.read(EEPROM_OPEN_ADDR);
    systemLocked  = (bool)EEPROM.read(EEPROM_LOCKED_ADDR);

    EEPROM.get(EEPROM_CHECKSUM_ADDR, storedChecksum);

    currentChecksum = calculateChecksum();
    return (currentChecksum == storedChecksum);
}


void initEEPROM() {
    
    EEPROM.update(EEPROM_MAGIC_ADDR,     MAGIC_BYTE_0);
    EEPROM.update(EEPROM_MAGIC_ADDR + 1, MAGIC_BYTE_1);

  
    vote_A = vote_B = vote_C = vote_count = 0;
    for (int i = 0; i < NUM_VOTERS; i++) voted[i] = false;
    electionOpen = false;
    systemLocked = false;

    saveAllData();
}

bool isFirstBoot() {
    return (EEPROM.read(EEPROM_MAGIC_ADDR)     != MAGIC_BYTE_0 ||
            EEPROM.read(EEPROM_MAGIC_ADDR + 1) != MAGIC_BYTE_1);
}


void resetElectionData() {
    vote_A = vote_B = vote_C = vote_count = 0;
    for (int i = 0; i < NUM_VOTERS; i++) voted[i] = false;
    electionOpen = false;
    systemLocked = false;
    adminFailCount = 0;
    dataCorrupted  = false;
    saveAllData();
}


void showClosed() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("VOTING CLOSED");
    lcd.setCursor(0, 1);
    lcd.print("Please Wait");
}

void showHome() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ENTER VOTER ID");
}

void showAdminMenu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("1.START 2.END");
    lcd.setCursor(0, 1);
    lcd.print("3.RESULT 4.RESET");
}



int findID(String id) {
    for (int i = 0; i < NUM_VOTERS; i++) {
        if (validID[i] == id) return i;
    }
    return -1;
}


void buzzerAlert(int duration_ms) {
    digitalWrite(buzz, HIGH);
    digitalWrite(led,  HIGH);
    delay(duration_ms);
    digitalWrite(led,  LOW);
    digitalWrite(buzz, LOW);
}


void setup() {
    pinMode(btnA,          INPUT_PULLUP);
    pinMode(btnB,          INPUT_PULLUP);
    pinMode(btnC,          INPUT_PULLUP);
    pinMode(resume_button, INPUT_PULLUP);
    pinMode(mainAdmin,     INPUT_PULLUP);
    pinMode(confirm,       INPUT_PULLUP);
    pinMode(led,  OUTPUT);
    pinMode(buzz, OUTPUT);

    lcd.init();
    lcd.backlight();
    Serial.begin(9600);

    
    if (isFirstBoot()) {
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("FIRST BOOT");
        lcd.setCursor(0, 1);
        lcd.print("INIT EEPROM...");
        delay(1500);
        initEEPROM();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("EEPROM READY");
        delay(1000);

    } else {
        
        bool ok = loadAllData();

        if (!ok) {
            
            dataCorrupted = true;
            currentState  = STATE_DATA_CORRUPTED;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("DATA CORRUPTED");
            lcd.setCursor(0, 1);
            lcd.print("ADMIN RESET REQ");
            
            return;
        }

       
        // if (systemLocked) {
        //     currentState = STATE_SYSTEM_LOCKED;
        //     lcd.clear();
        //     lcd.setCursor(0, 0);
        //     lcd.print("SYSTEM LOCKED");
        //     lcd.setCursor(0, 1);
        //     lcd.print("CONTACT ADMIN");
        //     return;
        // }
    }

    // Restore election status from EEPROM
    if (electionOpen) {
        currentState = STATE_IDLE;
        showHome();
    } else {
        currentState = STATE_VOTING_CLOSED;
        showClosed();
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  MAIN LOOP
// ═══════════════════════════════════════════════════════════════════════

void loop() {

    
    if (currentState == STATE_DATA_CORRUPTED) {
       
        digitalWrite(buzz, HIGH);
        delay(300);
        digitalWrite(buzz, LOW);
        delay(200);

        
        if (digitalRead(mainAdmin) == LOW) {
            while (digitalRead(mainAdmin) == LOW);
            digitalWrite(buzz, LOW);
            enteredPassword = "";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("ADMIN PASSWORD");
            currentState = STATE_ADMIN_LOGIN;
        }
        return;
    }

    // ── SYSTEM LOCKED: block admin login 
    // if (currentState == STATE_SYSTEM_LOCKED) {
    //     buzzerAlert(200);
    //     delay(800);
    //     // Only a hardware-level power cycle + manual EEPROM reset can
    //     // unlock (handled by DATA CORRUPTED path after reset).
    //     return;
    // }

    if (digitalRead(mainAdmin) == LOW) {
        while (digitalRead(mainAdmin) == LOW);

        // if (systemLocked) {
        //     lcd.clear();
        //     lcd.setCursor(0, 0);
        //     lcd.print("SYSTEM LOCKED");
        //     buzzerAlert(1000);
        //     return;
        // }

        enteredPassword = "";
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ADMIN  PASSWORD");
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
                    String mask = "";
                    for (unsigned int i = 0; i < enteredPassword.length(); i++) mask += '*';
                    lcd.print(mask);
                }

                if (key == '#') {
                    if (enteredPassword == adminPassword) {
                        adminFailCount = 0;
                        currentState   = STATE_ADMIN_MENU;
                        showAdminMenu();

                    } else {
                        // Wrong password
                        adminFailCount++;
                        lcd.clear();
                        lcd.setCursor(0, 0);
                        lcd.print("ACCESS DENIED");
                        lcd.setCursor(0, 1);
                        lcd.print("ATTEMPT ");
                        lcd.print(adminFailCount);
                        lcd.print("/3");
                        buzzerAlert(1000);
                        delay(3000);

                        if (adminFailCount >= 3) {
                            // Lock the system after 3 wrong attempts
                            systemLocked = true;
                            saveAllData();         
                            lcd.clear();
                            lcd.setCursor(0, 0);
                            lcd.print("SYSTEM LOCKED");
                            lcd.setCursor(0, 1);
                            lcd.print("3 WRONG TRIES");
                            buzzerAlert(2000);
                            delay(2000);
                            currentState = STATE_SYSTEM_LOCKED;
                        } else {
                            enteredPassword = "";
                            currentState    = STATE_VOTING_CLOSED;
                            showClosed();
                        }
                    }
                }

                if (key == '*') {
                    
                    enteredPassword = "";
                    currentState    = STATE_VOTING_CLOSED;
                    showClosed();
                }
            }
            break;
        }

        // ── ADMIN MENU ─
        case STATE_ADMIN_MENU: {
            if (key == '1') {
                // Start election
                electionOpen = true;
                saveAllData();          // persist open state
                currentState = STATE_IDLE;
                enteredID    = "";
                showHome();
            }
            if (key == '2') {
                // End election
                electionOpen = false;
                saveAllData();          // persist closed state
                currentState = STATE_VOTING_CLOSED;
                showClosed();
            }
            if (key == '3') {
                resultDisplayed = false;
                currentState    = STATE_SHOW_RESULT;
            }
            if (key == '4') {
                // Reset option – ask for confirmation
                currentState = STATE_RESET_CONFIRM;
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("RESET DATA?");
                lcd.setCursor(0, 1);
                lcd.print("#=YES   *=NO");
            }
            break;
        }

        // ── RESET CONFIRMATION ─
        case STATE_RESET_CONFIRM: {
            if (key == '#') {
                // Confirmed – wipe everything
                resetElectionData();    // clears RAM + writes clean EEPROM
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("DATA RESET DONE");
                lcd.setCursor(0, 1);
                lcd.print("ALL VOTES CLEAR");
                buzzerAlert(500);
                delay(2000);
                currentState = STATE_VOTING_CLOSED;
                showClosed();
            }
            if (key == '*') {
                // Cancelled
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("RESET CANCELLED");
                delay(1500);
                currentState = STATE_ADMIN_MENU;
                showAdminMenu();
            }
            break;
        }

        // ── IDLE (voter ID entry) ──────
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
                    // Clear entry
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

        // ── SELECT CANDIDATE ───
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
                lcd.print("PRESS CONFIRM");

                if (digitalRead(confirm) == LOW) {
                    while (digitalRead(confirm) == LOW);

                    // Record the vote
                    voted[currentVoterIndex] = true;
                    vote_count++;
                    if (selectedCandidate == 'A') vote_A++;
                    if (selectedCandidate == 'B') vote_B++;
                    if (selectedCandidate == 'C') vote_C++;

                    // SAVE TO EEPROM immediately 
                    saveAllData();
                  
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print("VOTE CANDIDATE:");
                    lcd.setCursor(7, 1);
                    lcd.print(selectedCandidate);

                    buzzerAlert(1000);
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
            buzzerAlert(1000);
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
            buzzerAlert(3000);
            enteredID        = "";
            currentVoterIndex = -1;
            showHome();
            currentState = STATE_IDLE;
            break;
        }

        
        case STATE_WAIT_ADMIN: {
            if (digitalRead(resume_button) == LOW) {
                while (digitalRead(resume_button) == LOW);
                enteredID         = "";
                currentVoterIndex = -1;
                selectedCandidate = '\0';
                currentState      = STATE_IDLE;
                showHome();
            }
            break;
        }

        // ── SHOW RESULT 
        case STATE_SHOW_RESULT: {
            if (!resultDisplayed) {
                // Reload checksum from EEPROM for display (verification)
                uint32_t cs;
                EEPROM.get(EEPROM_CHECKSUM_ADDR, cs);

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("TOT:");
                lcd.print(vote_count);
                lcd.print(" CS:");
                
                char csBuf[5];
                sprintf(csBuf, "%04lX", cs & 0xFFFF);
                lcd.print(csBuf);

                lcd.setCursor(0, 1);
                lcd.print("A:");
                lcd.print(vote_A);
                lcd.print(" B:");
                lcd.print(vote_B);
                lcd.print(" C:");
                lcd.print(vote_C);

                resultDisplayed = true;

               
                Serial.print("CHECKSUM: 0x");
                Serial.println(cs, HEX);
                Serial.print("A="); Serial.print(vote_A);
                Serial.print(" B="); Serial.print(vote_B);
                Serial.print(" C="); Serial.print(vote_C);
                Serial.print(" TOT="); Serial.println(vote_count);
            }
            if (key == '#') {
                resultDisplayed = false;
                currentState    = STATE_ADMIN_MENU;
                showAdminMenu();
            }
            break;
        }

        
        case STATE_DATA_CORRUPTED:
        case STATE_SYSTEM_LOCKED:
            break;

    } 
}
