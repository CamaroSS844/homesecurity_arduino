/*

  The circuit:
 * LCD RS pin to digital pin  12 ----- 7
 * LCD Enable pin to digital pin  11 ----8
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin  4
 * LCD D6 pin to digital pin  3
 * LCD D7 pin to digital pin  2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

// include the library code:
#include <LiquidCrystal.h>
#include <string.h>
#include <Keypad.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
//Card UID: A9 91 1C 0D ----card
//Card UID: 91 75 D1 1D ----tag
#define RST_PIN         6          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
byte accessUID[4] = {0xA9, 0x91, 0x1C, 0x0D};
byte accessUID2[4] = {0x91, 0x75, 0xD1, 0x1D};

MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo myservo;  // create servo object to control a servo
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {0, 1, A5, A4}; 
byte colPins[COLS] = {A3, A2, A1, A0}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 5, 4, 3, 2);

String password;
String attempt, value;
int numberOfAttemptsLeft = 2;
int pos = 0;    // variable to store the servo position


void reset(){
  attempt = "";
  password = "";
  lcd.clear();
  lcd.begin(16, 2);
  lcd.print("Scan Card");
}

void openDoor(){
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    myservo.write(pos);              
    delay(15);                       
  }
  for (int i = 5; i > 0; i --){
    lcd.setCursor(0, 0);
    lcd.print("Door closes in ");
    lcd.setCursor(0, 1);
    lcd.print(i);
    delay(1000);
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              
    delay(15);                       
  }
  reset();
}

void response(){
  lcd.print(numberOfAttemptsLeft);
  lcd.print(" attempts left");
  numberOfAttemptsLeft --;
}

void securityLock(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Try again in ");
  for (int i = 5; i > 0; i --){
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print(" mins");
    delay(5000);
  }
  reset();
  numberOfAttemptsLeft = 2;
  password = "";
}

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Scan Card");
  myservo.attach(9);

  SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522
	delay(4);
}

void loop() {
  if (password.length() == 0){
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
	  	return;
	  }

	  // Select one of the cards
	  if ( ! mfrc522.PICC_ReadCardSerial()) {
	  	return;
	  }
    if (mfrc522.uid.uidByte[0] == accessUID[0] && mfrc522.uid.uidByte[1] == accessUID[1] && mfrc522.uid.uidByte[2] == accessUID[2] && mfrc522.uid.uidByte[3] == accessUID[3]) {
      lcd.clear();
      lcd.print("Taboka F ");
      password = "4567";
    }else if (mfrc522.uid.uidByte[0] == accessUID2[0] && mfrc522.uid.uidByte[1] == accessUID2[1] && mfrc522.uid.uidByte[2] == accessUID2[2] && mfrc522.uid.uidByte[3] == accessUID2[3]) {
      lcd.clear();
      lcd.print("Trinity N ");
      password = "5432";
    }
  }
  char customKey;

  if (password.length() > 0 ){
    customKey = customKeypad.getKey();
  }
  
  if (customKey){
    switch(customKey){
      case 'D':
        lcd.clear();
        if(attempt == password) { 
          lcd.print("Welcome");
          openDoor();
          password = "";
        }else{
          (numberOfAttemptsLeft != 0)?
            response():
            securityLock();
        }
        attempt = "";
        break;
      case 'C':
        reset();
        break;
      case 'B':
        attempt = attempt.substring(0, (attempt.length() - 1));
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan Card");
        lcd.setCursor(0, 1);
        lcd.print(attempt);
        break;
      default:
        lcd.setCursor(0, 1);
        attempt = attempt + customKey;
        lcd.print(attempt);
        break;
    }
  }

  mfrc522.PICC_HaltA();
}

