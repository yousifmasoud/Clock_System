#include <Wire.h>  // Include Wire library for I2C communication

// Time variables
int year;
int month;
int day;
int hour;
int minutes;
int seconds;

// Alarm variables
int alarmHour = 0;
int alarmMinute = 0;
bool alarmSet = false;
bool alarmTriggered = false;
unsigned long alarmStartMillis = 0;

// Stopwatch variables
volatile bool stopwatchRunning = false;
unsigned long stopwatchStart = 0;
unsigned long stopwatchElapsed = 0;
bool displayStopwatch = false;
unsigned long displayStartMillis = 0;

// Time to show alarm settings on LCD when a button is pressed
const unsigned long displayDuration = 5000; // 5 seconds
unsigned long lastButtonPress = 0;

void setup() {
  // Initialize button pins as input with internal pull-up resistor
  DDRB &= ~(1 << PB1);  // Set PB1 (pin 9) as input
  PORTB |= (1 << PB1);  // Enable pull-up resistor on PB1

  DDRB &= ~(1 << PB2);  // Set PB2 (pin 10) as input
  PORTB |= (1 << PB2);  // Enable pull-up resistor on PB2

  DDRD &= ~(1 << PD2);  // Set PD2 (pin 2) as input
  PORTD |= (1 << PD2);  // Enable pull-up resistor on PD2

  // Initialize LED and buzzer pins
  DDRD |= (1 << PD7);   // Set PD7 (pin 7) as output
  DDRB |= (1 << PB0);   // Set PB0 (pin 8) as output

  // Initialize LCD pins
  DDRD |= (1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD3);  // Set PD6, PD5, PD4, PD3 as outputs (pins 6, 5, 4, 3)
  DDRB |= (1 << PB4) | (1 << PB3);  // Set PB4 and PB3 as outputs (pins 12 and 11)

  // Initialize I2C
  Wire.begin();
  
  // Initialize LCD
  lcdInit();

  // Configure external interrupt on pin 2 (INT0)
  EICRA |= (1 << ISC01);  // Trigger on falling edge of INT0 (PD2)
  EIMSK |= (1 << INT0);   // Enable INT0 interrupt

  // Enable global interrupts
  sei();

  // Set the RTC time to the current date and time
  setRTCTime();
}

void loop() {
  readRTC();
  
  bool buttonPressed = false;

  // Check if buttons are pressed to set the alarm
  if (!(PINB & (1 << PB1))) {  // Check if button on pin 9 (PB1) is pressed
    alarmHour++;
    if (alarmHour > 23) alarmHour = 0;
    lastButtonPress = millis(); // Update the last button press time
    buttonPressed = true;
    _delay_ms(200);  // Debounce _delay_ms
  }
  if (!(PINB & (1 << PB2))) {  // Check if button on pin 10 (PB2) is pressed
    alarmMinute++;
    if (alarmMinute > 59) alarmMinute = 0;
    lastButtonPress = millis(); // Update the last button press time
    buttonPressed = true;
    _delay_ms(200);  // Debounce _delay_ms
  }

  // Set the alarm when a button is pressed
  if (buttonPressed) {
    alarmSet = true;
  }

  // Display alarm time if a button was pressed or within the display duration
  if (buttonPressed || (lastButtonPress && (millis() - lastButtonPress) < displayDuration)) {
    displayAlarmTime();
  } else {
    // Display current date and time on LCD
    displayDateTime();
  }

  // Check if the current time matches the alarm time
  if (alarmSet && hour == alarmHour && minutes == alarmMinute && seconds == 0) {
    alarmTriggered = true;
    alarmStartMillis = millis(); // Record the start time of the alarm
  }

  // Trigger the alarm for 30 seconds
  if (alarmTriggered) {
    if (millis() - alarmStartMillis < 30000) { // Check if 30 seconds have passed
      PORTB |= (1 << PB0);  // Turn on buzzer
      _delay_ms(1000);
      PORTB &= ~(1 << PB0); // Turn off buzzer
      _delay_ms(400);
    } else {
      PORTB &= ~(1 << PB0); // Ensure the buzzer is off
      alarmTriggered = false; // Reset the alarm
      alarmSet = false; // Reset the alarm set flag
    }
  } else {
    PORTB &= ~(1 << PB0); // Ensure the buzzer is off when not triggered
  }

  // Stopwatch logic
  if (stopwatchRunning) {
    stopwatchElapsed = millis() - stopwatchStart;
    displayStopwatchTime();
  } else if (displayStopwatch) {
      if (millis() - displayStartMillis >= 6000) {
        displayStopwatch = false;
        displayDateTime();
      }
      else
        displayStopwatchTime();
  }

  _delay_ms(100); // Update every 100 ms
}

ISR(INT0_vect) {
  if (stopwatchRunning) {
    // Stop the stopwatch
    stopwatchRunning = false;
    displayStopwatch = true;
    displayStartMillis = millis();
    PORTD &= ~(1 << PD7);  // Turn off LED
    // PORTB |= (1 << PB0);  // Turn on buzzer
    // _delay_ms(500);  // Buzzer on for 0.5 seconds
    // PORTB &= ~(1 << PB0); // Turn off buzzer
  } else {
    // Start the stopwatch
    stopwatchRunning = true;
    stopwatchStart = millis();
    PORTD |= (1 << PD7);  // Turn on LED
    // PORTB |= (1 << PB0);  // Turn on buzzer
    // _delay_ms(500);  // Buzzer on for 0.5 seconds
    // PORTB &= ~(1 << PB0); // Turn off buzzer
  }
}

// The rest of the LCD code remains unchanged

void lcdCommand(char cmd) {
  PORTB &= ~(1 << PB4);  // rs = LOW --> Treat as Command
  lcdWrite4Bits(cmd >> 4); // write upper 4 bits
  lcdWrite4Bits(cmd); //write lower 4 bits
}

void lcdWrite(char value) {
  PORTB |= (1 << PB4);  // rs = HIGH --> Treat as Char Data
  lcdWrite4Bits(value >> 4);
  lcdWrite4Bits(value);
}

void lcdWrite4Bits(char value) {
  if (value & 0x01) PORTD |= (1 << PD6); else PORTD &= ~(1 << PD6);  // d4
  if (value & 0x02) PORTD |= (1 << PD5); else PORTD &= ~(1 << PD5);  // d5
  if (value & 0x04) PORTD |= (1 << PD4); else PORTD &= ~(1 << PD4);  // d6
  if (value & 0x08) PORTD |= (1 << PD3); else PORTD &= ~(1 << PD3);  // d7
  
  PORTB |= (1 << PB3);  // en = HIGH
  delayMicroseconds(1);
  PORTB &= ~(1 << PB3);  // en = LOW
  delayMicroseconds(100);
}

void lcdInit() {
  lcdCommand(0x33); // init
  lcdCommand(0x32); // 4-bit mode
  lcdCommand(0x28); // 4-bit mode, 2 lines, and 5x7 matrix
  lcdCommand(0x0C); // turns the display on, turns the cursor off, and turns the blinking off.
  lcdCommand(0x06); // configures the LCD to increment the cursor position and not to shift the display when a character is written
  lcdCommand(0x01); // clears the display screen and returns the cursor to the home position (top-left corner)
  _delay_ms(5);
}

void lcdClear() {
  lcdCommand(0x01);
  _delay_ms(2);
}

void lcdSetCursor(char col, char row) {
  static const char rowOffsets[] = { 0x00, 0x40, 0x14, 0x54 };
  lcdCommand(0x80 | (col + rowOffsets[row])); // set the DDRAM (Display Data RAM) address
  // 0x80 is the base address and it ORed with the address of the target cell
}

void lcdPrint(const char *str) {
  while (*str) {
    lcdWrite(*str++);
  }
}

void displayDateTime() {
  lcdClear();
  lcdSetCursor(0, 0);
  if (month < 10) lcdPrint(" ");
  lcdPrint(String(month).c_str());
  lcdPrint("/");
  if (day < 10) lcdPrint(" ");
  lcdPrint(String(day).c_str());
  lcdPrint("/");
  lcdPrint(String(year).c_str());

  lcdSetCursor(0, 1);
  if (hour < 10) lcdPrint(" ");
  lcdPrint(String(hour).c_str());
  lcdPrint(":");
  if (minutes < 10) lcdPrint("0");
  lcdPrint(String(minutes).c_str());
  lcdPrint(":");
  if (seconds < 10) lcdPrint("0");
  lcdPrint(String(seconds).c_str());
}

void displayAlarmTime() {
  lcdClear();
  lcdSetCursor(0, 0);
  lcdPrint("Set Alarm Time:");
  
  lcdSetCursor(0, 1);
  if (alarmHour < 10) lcdPrint("0");
  lcdPrint(String(alarmHour).c_str());
  lcdPrint(":");
  if (alarmMinute < 10) lcdPrint("0");
  lcdPrint(String(alarmMinute).c_str());
}

void displayStopwatchTime() {
  unsigned long elapsed = stopwatchElapsed;
  unsigned long hours = elapsed / 3600000;
  elapsed %= 3600000;
  unsigned long minutes = elapsed / 60000;
  elapsed %= 60000;
  unsigned long seconds = elapsed / 1000;
  unsigned long millis = elapsed % 1000;

  lcdClear();
  lcdSetCursor(0, 0);
  lcdPrint("Stopwatch:");

  lcdSetCursor(0, 1);
  if (hours < 10) lcdPrint("0");
  lcdPrint(String(hours).c_str());
  lcdPrint(":");
  if (minutes < 10) lcdPrint("0");
  lcdPrint(String(minutes).c_str());
  lcdPrint(":");
  if (seconds < 10) lcdPrint("0");
  lcdPrint(String(seconds).c_str());
  lcdPrint(".");
  if (millis < 100) lcdPrint("0");
  if (millis < 10) lcdPrint("0");
  lcdPrint(String(millis).c_str());
}

void setRTCTime() {
  Wire.beginTransmission(0x68); // DS3231 I2C address
  Wire.write(0); // Start at register 0

  // Set the date and time here
  Wire.write(decToBcd(0));    // Seconds
  Wire.write(decToBcd(15));    // Minutes
  Wire.write(decToBcd(14));   // Hours (24-hour format)
  Wire.write(decToBcd(1));    // Day of week (1-7)
  Wire.write(decToBcd(20));    // Date (1-31)
  Wire.write(decToBcd(5));    // Month (1-12)
  Wire.write(decToBcd(24));   // Year (last two digits)

  Wire.endTransmission();
}

char decToBcd(char val) {
  return (val / 10 * 16) + (val % 10);
}

void readRTC() {
  Wire.beginTransmission(0x68); // DS3231 I2C address
  Wire.write(0); // Start at register 0
  Wire.endTransmission();
  
  Wire.requestFrom(0x68, 7); // Request seven bytes (seconds, minutes, hours, day, date, month, year)
  
  seconds = bcdToDec(Wire.read() & 0x7F);
  minutes = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read() & 0x3F);
  Wire.read(); // Skip day of the week
  day = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read()) + 2000;
}

char bcdToDec(char val) {
  return (val / 16 * 10) + (val % 16);
}
