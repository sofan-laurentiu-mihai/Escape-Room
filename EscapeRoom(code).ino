#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>  // Include the Keypad library

// Initialize the LCD (address: 0x27, columns: 16, rows: 2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions for buttons, LED, ultrasonic sensor, and LDR
const int button1Pin = 2;  // Button 1 connected to pin 2
const int button2Pin = 4;  // Button 2 connected to pin 4
const int ledPin = 8;      // LED connected to pin 8
const int trigPin = 6;     // Ultrasonic sensor Trig pin
const int echoPin = 7;     // Ultrasonic sensor Echo pin
const int ldrPin = A0;     // Photoresistor connected to analog pin A0

// Keypad configuration
const byte ROW_NUM    = 4;    // four rows
const byte COLUMN_NUM = 4;    // four columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte pin_rows[ROW_NUM] = {1,3,5,9};  // Row pins connected to 13, 12, 11, 10
byte pin_column[COLUMN_NUM] = {10,11,12,13};  // Column pins connected to 9, 5, 3, 1
/*
byte pin_rows[ROW_NUM] = {9, 5, 3, 1};  // Row pins connected to 13, 12, 11, 10
byte pin_column[COLUMN_NUM] = {13, 12, 11, 10};  // Column pins connected to 9, 5, 3, 1
*/

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);  // Initialize keypad

// Variables for button presses, LED, ultrasonic sensor, and light intensity
int button1State = 0;      // Variable to store button 1 state
int lastButton1State = 0;  // To detect button 1 press
int button2State = 0;      // Variable to store button 2 state
int lastButton2State = 0;  // To detect button 2 press
int pressCount = 0;        // Count of button 1 presses
bool isDisplayed = false;  // Flag to indicate if the number has been displayed

// Store the random numbers for code
int code[4];  // Array to store the 4-digit code
int enteredCode[4];  // Array to store the entered code
int enteredIndex = 0;  // Index to store the digits in the entered code

void setup() {
  lcd.begin(16, 2);            // Initialize the LCD with 16 columns and 2 rows
  lcd.backlight();             // Turn on the LCD backlight
  lcd.print("Press Button");   // Initial message
  pinMode(button1Pin, INPUT_PULLUP); // Set button 1 pin as input with pull-up resistor
  pinMode(button2Pin, INPUT_PULLUP); // Set button 2 pin as input with pull-up resistor
  pinMode(ledPin, OUTPUT);     // Set LED pin as output
  pinMode(trigPin, OUTPUT);    // Set Trig pin as output
  pinMode(echoPin, INPUT);     // Set Echo pin as input
  randomSeed(analogRead(0));   // Seed for random numbers
}

void loop() {
  // Read button 1 state
  button1State = digitalRead(button1Pin);

  // Check for button 1 press
  if (button1State == LOW && lastButton1State == HIGH) { // Button 1 pressed
    pressCount++;
    delay(50); // Debounce delay
  }
  lastButton1State = button1State; // Update last button 1 state

  if (pressCount == 4 && !isDisplayed) {
    lcd.clear();
    code[0] = random(0, 9); // Generate a random number for code[0] (from button press)
    lcd.print("Random Number:");
    lcd.setCursor(0, 1);
    lcd.print(code[0]);
    delay(5000); // Show number for 5 seconds
    lcd.clear();
    pressCount = 0;  // Reset the press count
    isDisplayed = true; // Mark as displayed
  } else if (button1State == HIGH && isDisplayed) {
    isDisplayed = false; // Reset display flag after release
  }

  // Read button 2 state
  button2State = digitalRead(button2Pin);

  // Check for button 2 press
  if (button2State == LOW && lastButton2State == HIGH) { // Button 2 pressed
    lcd.clear();
    lcd.print("Lighting LED...");
    int blinkCount = random(1, 9); // Generate a random number for the LED blink count
    code[1] = blinkCount;  // Store the number of blinks for code[1]
    for (int i = 0; i < blinkCount; i++) {
      digitalWrite(ledPin, HIGH); // Turn LED on
      delay(500);                // Wait for 500ms
      digitalWrite(ledPin, LOW); // Turn LED off
      delay(500);                // Wait for 500ms
    }
    lcd.clear();
    lcd.print("Done!Press again");
    delay(1000); // Short delay before resetting
  }
  lastButton2State = button2State; // Update last button 2 state

  // Measure distance with ultrasonic sensor
  long distance = measureDistance();

  // Check if distance is less than 3 cm
  if (distance > 0 && distance < 3) {
    lcd.clear();
    lcd.print("Dist < 3cm:");
    code[2] = random(0, 10); // Store the random number for code[2]
    lcd.setCursor(0, 1);
    lcd.print(code[2]);
    delay(5000); // Show number for 5 seconds
    lcd.clear();
  }

  // Read LDR value
  int ldrValue = analogRead(ldrPin);

  // Check if light intensity exceeds threshold
  if (ldrValue > 100) { // Adjust threshold based on your lighting conditions
    lcd.clear();
    lcd.print("Light Detected!");
    code[3] = random(0, 9); // Store the random number for code[3]
    lcd.setCursor(0, 1);
    lcd.print(code[3]);
    delay(5000); // Show number for 5 seconds
    lcd.clear();
  }

  // Read the keypad input
  char key = keypad.getKey();
  if (key) {
    // If key is pressed, store it in enteredCode
    if (enteredIndex < 4) {
      enteredCode[enteredIndex] = key - '0';  // Convert the key to integer and store it
      enteredIndex++;  // Move to the next index

      // Display the "Entered" message and show the current input
      lcd.clear();
      lcd.print("Entered: ");
      for (int i = 0; i < enteredIndex; i++) {
        lcd.print(enteredCode[i]);
      }
    }

    // Check if all 4 digits are entered
    if (enteredIndex == 4) {
      // Check if the entered code matches the generated code
      if (enteredCode[0] == code[0] && enteredCode[1] == code[1] && enteredCode[2] == code[2] && enteredCode[3] == code[3]) {
        lcd.clear();
        lcd.print("You escaped!");
      } else {
        lcd.clear();
        lcd.print("Try again");
      }
      delay(2000);  // Display result for 2 seconds

      // Reset the entered code for next attempt
      enteredIndex = 0;
      lcd.clear();
      lcd.print("Press Button");
    }
  }
}

// Function to measure distance using ultrasonic sensor
long measureDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH); // Measure the time the echo is HIGH
  long distance = duration * 0.034 / 2;   // Calculate distance in cm
  return distance;
}
