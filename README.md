# Electronic Voting Machine (EVM)

## Overview

This project implements an Arduino-based Electronic Voting Machine (EVM) designed to provide a secure and reliable voting process. The system authenticates voters, prevents duplicate voting, allows administrative control, records votes, displays results, and provides audio-visual feedback to users.

## Features

* Voter ID Authentication
* Duplicate Vote Prevention
* Candidate Selection
* Vote Confirmation
* Voting Pause and Resume Control
* Administrative Authentication
* Voting Start and Stop Control
* Vote Counting
* Result Display
* Checksum Generation
* LCD-Based User Interface
* LED Status Indication
* Buzzer Feedback System
* Invalid Voter Alerts
* Vote Confirmation Alerts
* Administrative Access Control

## Components Used

* Arduino Uno
* 4x4 Matrix Keypad
* 16x2 I2C LCD Display
* Push Buttons
* LEDs
* Buzzer
* Breadboard
* Jumper Wires

## Working Principle

1. The administrator authenticates using predefined passwords.
2. The administrator starts the voting process through the admin menu.
3. Voters enter their voter ID using the keypad.
4. The system verifies the voter ID against the stored database.
5. Invalid or previously used IDs are rejected with LED and buzzer indications.
6. Valid voters are allowed to select a candidate.
7. The voter confirms the selected candidate.
8. The vote is recorded and counted.
9. LED and buzzer feedback confirm successful voting.
10. Voting pauses until the administrator resumes the process.
11. The administrator can view results or stop voting through the admin menu.
12. A checksum is generated for basic vote integrity verification.

## Future Enhancements

* EEPROM-Based Vote Storage
* Data Integrity Verification
* Persistent Vote Recovery After Power Loss
* Enhanced Security Features
* Audit Log Generation

## Author

Tushar Kanti Sahariah
