# Electronic Voting Machine (EVM)

## Overview

This project implements an Arduino-based Electronic Voting Machine (EVM) designed to provide a secure and reliable voting process. The system authenticates voters, prevents duplicate voting, records votes, and provides user feedback through an LCD interface.

## Features

* Candidate Selection
* Vote Confirmation
* Timeout Handling
* Voter ID Authentication
* Invalid ID Detection
* Duplicate Vote Prevention

## Components Used

* Arduino Uno
* 16x2 LCD Display (I2C)
* 4x4 Keypad
* Push Buttons
* LEDs
* Buzzer
* Breadboard and Jumper Wires

## Working Principle

1. The voter enters their voter ID using the keypad.
2. The system verifies the entered ID against the stored voter database.
3. Invalid IDs are rejected.
4. Previously used IDs are blocked from voting again.
5. Valid voters are allowed to select a candidate.
6. The voter confirms the selected candidate.
7. The vote is recorded and the voter session ends.

## Project Structure

```text
EVM-Project/
│
├── README.md
│
├── src/
│   └── EVM.ino
│
└── tinkercad_stages/
    ├── Stage_1.png
    └── Stage_2.png
```

## Future Enhancements

* Administrative Control Panel
* Vote Result Display
* EEPROM-Based Vote Storage
* Data Integrity Verification
* Enhanced User Feedback

## Author

Tushar Kanti Sahariah
<<<<<<< HEAD
=======
B.Tech, Electronics & Telecommunication Engineering
Assam Engineering College
>>>>>>> efab62c582536ce9401561e62f509d03eff5412e
