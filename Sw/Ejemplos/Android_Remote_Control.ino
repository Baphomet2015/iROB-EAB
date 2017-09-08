//    Android remote control v1.0
//      This program was created by Project Biped to let an Android device
//    to send commands to an Arduino board to control the robot.
//    Use it as the starting point for your projects!
//
//    Copyright (C) 2012  Jonathan Dowdall, Project Biped (www.projectbiped.com)
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include <SPI.h>
#include <Adb.h>
#include <Servo.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Constants
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int maximumServoShieldPosition = 2200;   // the maximum pulse duration for the servo shield (2ms pulse)
const int minimumServoShieldPosition = 800;   // the minimum pulse duration for the servo shield (1ms pulse)
const int numberOfServos             = 12;      // the number of servos
const int numberOfJoints             = 12;
const int numberOfFrames             = 24;
const int pingPin                    = 4;

int servoPins[numberOfServos] = {22, 24, 26, 28, 30, 32, 38, 40, 42, 44, 46, 48};  // the pin for each servo 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Variables
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Servo servos[numberOfServos];  // create servo object to control a servo 
Connection * connection;       // Adb connection.
long lastTime;                 // Elapsed time for ADC sampling

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handler for the shell connection. 
void adbEventHandler(Connection * connection, adb_eventType event, uint16_t length, uint8_t * data)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // Data packets contain two bytes, one for each servo, in the range of [0..180]
  if (event == ADB_CONNECTION_RECEIVE)
    for(int servo = 0; servo < numberOfServos; servo++)
    {
      // each servo position is sent as a 2 byte value (high byte, low byte) integer (from -32,768 to 32,767)
      // this number is encoding the angle of the servo. The number is 100 * the servo angle.  This allows for the
      // storage of 2 significant digits(i.e. the value can be from -60.00 to 60.00 and every value in between).
      // Also remember that the servos have a range of 120 degrees. The angle is written in positions
      // which range from a minimum of 800 (-60 degrees) and go to a maximum of 2200 (60 degrees)
      //word value = word(inputBuffer[servo*2 + 1 + 3], inputBuffer[servo*2 + 0 + 3]);      
      int value = word(data[servo*2 + 1], data[servo*2 + 0]);      
      
      // flip for the left leg.
      if(servo >= numberOfServos/2)
        value = map(value, -6000,6000,6000,-6000);
      
      servos[servo].write(map(value, -6000,6000,800,2200));              // tell servo to go to position in variable 'pos' 
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{

  // Initialise serial port
  Serial.begin(57600);
  
  // Note start time
  lastTime = millis();
  
  InitializeServos();
  
  // Initialise the ADB subsystem.  
  ADB::init();

  // Open an ADB stream to the phone's shell. Auto-reconnect
  connection = ADB::addConnection("tcp:4567", true, adbEventHandler);  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void InitializeServos()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{   
  // Assign the correct pin to each servo.
  for(int s = 0; s < numberOfServos; s++)
    servos[s].attach(servoPins[s]);  
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // Check to see if a new measurement should be taken.
  if ((millis() - lastTime) > 100)
  {
    // Read the distance (in cm) from the range sensor.
    uint16_t distance = MeasureDistance();
    
    // Send it to the Android device.
    connection->write(2, (uint8_t*)&distance);
    
    // Remember the time stamp.
    lastTime = millis();
  }

  // Poll the ADB subsystem.
  ADB::poll();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// some of the code below was found on http://arduino.cc/en/Tutorial/Ping?from=Tutorial.UltrasoundSensor.  Thanks Arduino guys!
//  credit:
//    by David A. Mellis
//    modified 30 Aug 2011
//    by Tom Igoe
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long MeasureDistance()
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  // convert the time into a distance
  cm = microsecondsToCentimeters(duration);

  return cm;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long microsecondsToCentimeters(long microseconds)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}
