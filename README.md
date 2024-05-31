# Arduino Timekeeping Project

## Overview

This Arduino project provides functionalities for displaying the current time, setting alarms, and using a stopwatch. It utilizes I2C communication for interfacing with various components and includes button inputs for user interaction.

## Features

- **Time Display**: Keeps track of the current time including year, month, day, hour, minutes, and seconds.
- **Alarm**: Allows the user to set an alarm that triggers at a specified time.
- **Stopwatch**: Includes a stopwatch feature that can be started, stopped, and reset by the user.

## Components

- Arduino board
- I2C compatible display (e.g., LCD)
- Buttons for user input

## Pin Configuration

- **Button 1**: Connected to pin 9
- **Button 2**: Connected to pin 10
- **Button 3**: Connected to pin 2

## Software Requirements

- Arduino IDE
- Wire library (for I2C communication)

## Installation

1. **Clone the repository**:
   ```sh
   git clone https://github.com/yourusername/your-repo-name.git
   ```
2. **Open the project**:
   Open the `FinalFinal.ino` file in the Arduino IDE.
3. **Upload to Arduino**:
   Connect your Arduino board to your computer and upload the code using the Arduino IDE.

## Usage

### Setup

1. **Initialize the Buttons**:
   The setup function configures the button pins as inputs with internal pull-up resistors.
   ```cpp
   DDRB &= ~(1 << PB1);  // Set PB1 (pin 9) as input
   PORTB |= (1 << PB1);  // Enable pull-up resistor on PB1

   DDRB &= ~(1 << PB2);  // Set PB2 (pin 10) as input
   PORTB |= (1 << PB2);  // Enable pull-up resistor on PB2

   DDRD &= ~(1 << PD2);  // Set PD2 (pin 2) as input
   PORTD |= (1 << PD2);  // Enable pull-up resistor on PD2
   ```

### Functions

- **Timekeeping**:
  The project keeps track of the current time using variables for year, month, day, hour, minutes, and seconds.

- **Alarm**:
  Users can set an alarm by specifying the hour and minute. The alarm status is indicated by `alarmSet` and `alarmTriggered` flags.

- **Stopwatch**:
  The stopwatch can be controlled by the user, with functions to start, stop, and reset. The `stopwatchRunning` flag indicates its current state.

### Interaction

- **Button 1**: Typically used for setting or interacting with the time.
- **Button 2**: Used for interacting with the alarm feature.
- **Button 3**: Used for controlling the stopwatch.

### Display

The time, alarm, and stopwatch information are displayed on an I2C compatible display.

## Contributing

Contributions are welcome! Please fork this repository and submit pull requests.

## Contact

For questions or suggestions, feel free to open an issue or contact [yousifmasoud8@gmail.com](mailto:your-email@example.com).

Copy the above content into your `README.md` file to provide a clear and structured overview of your Arduino project. If you need any modifications or additional sections, let me know!
