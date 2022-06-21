/*  Home Operating System
	Date Completed: 12/17/21
    By: Jerry Zhu
	The home operating system is an Arduino project that gets input from 
    a temperature sensor, light sensor, and gas sensor. 
    It processes this information and outputs to a DC motor, 
    LED, RGB LED, buzzer, and LCD display. 
    It functions as what a control and operating system would look like in a home. 
    Modes supported: 
    1 --> Display temperature reading
    2 --> Display photoresistor reading
    3 --> Display gas reading
    4 --> Change DC motor threshold from 10 to 40
    5 --> Change RGB LED threshold from 0 to 30
    6 --> Change hours from 1 to 12
    7 --> Change minutes from 0 to 59
    8 --> Change seconds from 0 to 59
    9 --> Change AM/PM 
    Buttons:
    Mode  --> Increments the current mode
    Left --> Decreases the desired value
    Right --> Increases the desired value
*/

#include <LiquidCrystal.h> // Import the liquid crystal library

// Initializes the LCD display
LiquidCrystal lcd(12, 11, 7, 4, 3, 2);

// Initialize variables
const int LEDPin = 13;
const int bluePin = 10;
const int greenPin = 9;
const int relayPin = 8;
const int redPin = 6;
const int buzzerPin = 5;
const int buttonPin = A0;
const int photoPin = A1;
const int tempPin = A2; 
const int gasPin = A3;
int sensorValue = 0;
int tempValue = 0;
int gasValue = 0;
int photoValue = 0;
int hour = 11;
int min = 59;
int sec = 59;
int mode = 1;
int lastMode = 0;
int fanThreshold = 28;
int LEDThreshold = 20;
String printValue = ""; 
String lastValue = "";
String timeStr = "";
bool pressed = false;
bool lastPressed = false; 
bool AM = true;

/**
* Function to read the analog input and return which button is pressed
* @return int      The button that is pressed
*/
int readLCDButtons(){                   // read the buttons
 int ADC_value = analogRead(buttonPin);   // buttons are all connected to analog input 0, in a voltage divider circuit
 if (ADC_value > 1000) return -1;         // this is the most likely result, so check it first
 else if (ADC_value < 250)  return 2;     // MODE
 else if (ADC_value < 600)  return 1;     // LEFT
 else if (ADC_value < 750)  return 0;     // RIGHT
 else return -1;                          // when all others checks fail, consider no key is pressed 
}

/**
* Function to keep a certain value in the range from low to high inclusive
* @param value      The value that should be kept in range
* @param high       The maximum value in the range
* @param low        The minimum value in the range
* @return int       The new value in the desired range 
*/
int check(int value, int low, int high){
  if (value < low){
    value = high;
  }
  if (value > high){
    value = low;
  }
  return value;
}

/**
* Setup function to be run once
*/
void setup()
{
  // Set pins to input or output
  pinMode(LEDPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(photoPin, INPUT);
  pinMode(tempPin, INPUT);
  pinMode(gasPin, INPUT);
  
  // Stop interrupts
  cli();

  // Set timer interrupt to 1Hz (one operation a second) by setting register fields
  TCCR1A = 0; // Set entire TCCR1A register to 0
  TCCR1B = 0; // Set entire TCCR1B register to 0
  TCNT1  = 0; // Initialize counter
  // Set compare match register for 1Hz increments
  OCR1A = 15624; // 15624 = (16*10^6) / (1*1024) - 1 
  // Turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for a prescaler down of 1024
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // Enable the timer interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Start interrupts
  sei();

  // initialize the LCD
  lcd.begin(16, 2);
}

/**
* Method for timer interrupt timer interrupt creates pulse wave of frequency 0.5kHz 
* Toggles high then toggles low
* During this interrupt time, the time is incremented
*/
ISR(TIMER1_COMPA_vect){
  // Increment number of seconds
  sec++;
  if (sec == 60){
    // Increment minutes
    sec = 0;
    min++;
    if (min == 60){
      // Increment hours
      min = 0;
      hour++;
      // Make sure hour is in range
      if (hour == 12){
        AM = !AM;
      }
      if (hour == 13){
        sec = 0;
        min = 0;
        hour = 1;
      }
    }
  }
}

void loop(){
  // Get the current value pressed
  int curButton = readLCDButtons();	
  pressed = curButton == -1 ? false : true;
  // Read in any sensor values
  sensorValue = analogRead(tempPin);
  float voltage = sensorValue * 5.0/1023;
  float degC = (voltage - 0.5)*100;
  int tempValue = (int)degC;
  gasValue = analogRead(gasPin);
  photoValue = analogRead(photoPin);
  // Increment the mode if mode button pressed 
  if (curButton == 0 && pressed != lastPressed){
    mode++;
    mode = check(mode, 1, 9);
  }
  if (mode == 1){ // Display temperature reading
    printValue = String(tempValue);
  }
  else if (mode == 2){ // Display photoresistor reading
    printValue = String(photoValue);
  }
  else if (mode == 3){ // Display gas sensor reading
    printValue = String(gasValue);
  }
  else if (mode == 4){ // Change the DC motor threshold
    if (pressed != lastPressed){
      if (curButton == 1){ // LEFT
        fanThreshold--;
        fanThreshold = check(fanThreshold, 10, 40);
      }
      else if (curButton == 2){ // RIGHT
        fanThreshold++;
        fanThreshold = check(fanThreshold, 10, 40);
      }
    }
    printValue = String(fanThreshold);
  }
  else if (mode == 5){ // Change the RGB LED threshold
    if (pressed != lastPressed){
      if (curButton == 1){ // LEFT
        LEDThreshold--;
        LEDThreshold = check(LEDThreshold, 0, 30);
      }
      else if (curButton == 2){ // RIGHT
        LEDThreshold++;
        LEDThreshold = check(LEDThreshold, 0, 30);
      }
    }
    printValue = String(LEDThreshold);
  }
  else if (mode == 6){ // Change the hour 
    if (pressed != lastPressed){
      if (curButton == 1){ // LEFT
        hour--;
        hour = check(hour, 1, 12);
      }
      else if (curButton == 2){ // RIGHT
        hour++;
        hour = check(hour, 1, 12);
      }
    }
    printValue = "Change hour";
  }
  else if (mode == 7){ // Change the minute
    if (pressed != lastPressed){
      if (curButton == 1){ // LEFT
        min--;
        min = check(min, 0, 59);
      }
      else if (curButton == 2){ // RIGHT
        min++;
        min = check(min, 0, 59);
      }
    }
    printValue = "Change min";
  }
  else if (mode == 8){ // Change the second
    if (pressed != lastPressed){
      if (curButton == 1){ // LEFT
        sec--;
        sec = check(sec, 0, 59);
      }
      else if (curButton == 2){ // RIGHT
        sec++;
        sec = check(sec, 0, 59);
      }
    }
    printValue = "Change sec";
  }
  else { // Change AM/PM value
    if (pressed != lastPressed){
      if (curButton == 1 || curButton == 2){
        AM = !AM;
      }
    }
    printValue = "Change AM/PM";
  }
    
  // Check to turn on the fan
  if (tempValue > fanThreshold){ 
      digitalWrite(relayPin, HIGH);
  }
  else {
      digitalWrite(relayPin, LOW);
  }
  // Check to turn on the RGB LED
  if (tempValue < LEDThreshold){
      // PURPLE COLOR: (128, 0, 128)
      digitalWrite(redPin, 128);
      digitalWrite(greenPin, 0);
      digitalWrite(bluePin, 128);
  }
  else {
      digitalWrite(redPin, 0);
      digitalWrite(greenPin, 0);
      digitalWrite(bluePin, 0);
  }
  // Check to turn on the buzzer and light
  if (gasValue > 50){
    tone(buzzerPin, 600);
    digitalWrite(LEDPin, HIGH);
  }
  else {
    noTone(buzzerPin);
    digitalWrite(LEDPin, LOW);
  }

  // Print the mode to the LCD display
  if (mode != lastMode){
    lcd.setCursor(0, 0);
    lcd.print("   ");
    lcd.setCursor(0, 0);
    lcd.print(mode);
  }

  // Print the desired value to the LCD
  if (mode != lastMode || printValue != lastValue){
    lcd.setCursor(4, 0);
    lcd.print("                      ");
    lcd.setCursor(4, 0);
    lcd.print(printValue);
  }
  
  // Create a String of the current time
  // String format: hour:minute:day AM/PM

  timeStr = "";

  if (hour < 10){
    timeStr += "0" + String(hour);
  }
  else {
    timeStr += String(hour);
  }

  timeStr += ":";

  if (min < 10){
    timeStr += "0" + String(min);
  }
  else {
    timeStr += String(min);
  }

  timeStr += ":";

  if (sec < 10){
    timeStr += "0" + String(sec);
  }
  else {
    timeStr += String(sec);
  }

  if (AM){
    timeStr += " AM";
  }
  else {
    timeStr += " PM";
  }

  // Display the time String to the LED
  lcd.setCursor(0, 1);
  lcd.print(timeStr);

  // Keep the last processed values for the next loop iteration    
  lastMode = mode;
  lastPressed = pressed;
  lastValue = printValue;
}