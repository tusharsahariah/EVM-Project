# Electronic Voting Machine (EVM)

## Overview

This project implements an Arduino-based Electronic Voting Machine (EVM) designed to provide a secure and reliable voting process. The system authenticates voters, prevents duplicate voting, records votes, and provides user feedback through an LCD interface.

## Features

* Candidate Selection
* Vote Confirmation
* Voter Authentication
* Duplicate Vote Prevention
* Voting Pause Control
* Voting Resume Function
* Administrative Authentication
* Voting Start and Stop Control
* Vote Counting
* Result Display
* Checksum Generation
* Invalid Voter Detection


## Components Used

* Arduino Uno
* 16x2 LCD Display (I2C)
* 4x4 Keypad
* Push Buttons
* LEDs
* Buzzer
* Breadboard and Jumper Wires

## Working Principle

1. Administrator authenticates using predefined passwords.
2. Administrator starts the voting process.
3. Voters enter their voter ID using the keypad.
4. The system verifies the voter ID against the stored database.
5. Invalid or previously used IDs are rejected.
6. Valid voters are allowed to select a candidate.
7. The vote is recorded and counted.
8. Voting pauses until the administrator resumes the process.
9. The administrator can stop voting or view results through the admin menu.
10. A checksum is generated for basic vote integrity verification.


## Future Enhancements

* EEPROM-Based Vote Storage
* Data Integrity Verification
* Buzzer-Based User Feedback
* Enhanced Error Handling
* Persistent Vote Recovery After Power Loss

## Author

Tushar Kanti Sahariah