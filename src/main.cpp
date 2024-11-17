#include <Arduino.h>
#include <Servo.h>

/**
 * @brief Reads the distance from an ultrasonic sensor.
 *
 * This function triggers the ultrasonic sensor and calculates the distance
 * based on the time it takes for the echo to return.
 *
 * @param trigPin The pin number used to trigger the ultrasonic sensor.
 * @param echoPin The pin number used to read the echo from the ultrasonic sensor.
 * @return long The distance measured in centimeters.
 */
long readUltrasonicDistance(int trigPin, int echoPin);

/**
 * @brief Displays a number on a 7-segment display.
 *
 * This function takes an integer input and activates the corresponding
 * segments of the display to show the number.
 *
 * @param num The integer number to be displayed (0-9).
 */
void displayNumber(int num);

// Button pins
const int buttonPin = 2;

// Light pins
const int greenPin = 3;
const int yellowPin = 4;
const int redPin = 5;

// Buzzer pin
const int buzzerPin = 6;

// Servo pin
const int servoPin = 7;

// Ultrasonic pins
const int trigPin = 8;
const int echoPin = 9;

// Segment pins
const int segmentA = 11;
const int segmentB = 10;
const int segmentF = 12;
const int segmentG = 13;
const int segmentE = A0;
const int segmentC = A2;
const int segmentD = A1;

// Create objects
Servo gate;

// Constants
const double minDistance = 25;
const int crossTime = 10 * 1000 - 1; // 10 seconds
const int gateUp = 150;
const int gateDown = 50;
const int numbers[11][7] = {
    {1, 1, 1, 1, 1, 1, 0}, // 0
    {0, 1, 1, 0, 0, 0, 0}, // 1
    {1, 1, 0, 1, 1, 0, 1}, // 2
    {1, 1, 1, 1, 0, 0, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1}, // 4
    {1, 0, 1, 1, 0, 1, 1}, // 5
    {1, 0, 1, 1, 1, 1, 1}, // 6
    {1, 1, 1, 0, 0, 0, 0}, // 7
    {1, 1, 1, 1, 1, 1, 1}, // 8
    {1, 1, 1, 1, 0, 1, 1}, // 9
    {0, 0, 0, 0, 0, 0, 0}, // off
};

// States
float crossStartTime = -crossTime;
bool pedState = false;
bool lightState = true;
bool buzzerState = false;
bool crossState = false;
bool dimmer = false;

// Variables
int distanceArray[10] = {100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

void setup()
{
  // Start serial
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  gate.attach(servoPin);
  gate.write(gateUp);

  // Set up segment
  pinMode(segmentA, OUTPUT);
  pinMode(segmentB, OUTPUT);
  pinMode(segmentC, OUTPUT);
  pinMode(segmentD, OUTPUT);
  pinMode(segmentE, OUTPUT);
  pinMode(segmentF, OUTPUT);
  pinMode(segmentG, OUTPUT);
}

void loop()
{
  // Read the button state and set queue
  if (digitalRead(buttonPin) == 0)
  {
    Serial.print("{Button PRESS} ");
    pedState = true;
  }

  // Read the distance
  long distance = readUltrasonicDistance(trigPin, echoPin);
  Serial.print("{Distance ");
  Serial.print(distance);
  Serial.print("} ");

  // Apply filter
  for (int i = 9; i > 0; i--)
  {
    distanceArray[i] = distanceArray[i - 1];
  }
  distanceArray[0] = distance;

  // Calculate the average of the distanceArray
  long sum = 0;
  for (int i = 0; i < 10; i++)
  {
    sum += distanceArray[i];
  }
  float avgDistance = sum / 10.0; // Get the average distance

  // Timer for crossing
  long crossTimer = crossStartTime + crossTime - millis();
  if (crossTimer < -crossTime)
    crossTimer = -crossTime;

  // Set cross state
  if (crossTimer >= 0)
    crossState = true;
  else
    crossState = false;

  // Start the crossing
  if (pedState)
  {
    dimmer = false;
    // Check if safe to cross
    if (avgDistance >= minDistance)
    {
      crossStartTime = millis();
      lightState = false;
      pedState = false;
    }
    else
    {
      lightState = true;
      buzzerState = true;
    }
  }
  else if (!pedState && !crossState)
  {
    buzzerState = false;
    lightState = true;
    if (crossTimer == -crossTime)
      dimmer = true;
  }

  // Pedestrian is crossing
  if (crossState)
  {
    gate.write(gateDown);          // Set the servo into down position
    digitalWrite(yellowPin, HIGH); // Set yellow lights
    if (avgDistance >= minDistance)
    {
      buzzerState = false;
      lightState = false;
    }
    else
    {
      buzzerState = true;
      lightState = true;
    }
    // Display countdown to segment
    int segNumber = (int)(crossTimer / 1000);
    displayNumber(segNumber);
  }
  else
  {
    gate.write(gateUp);           // Move the gate up
    digitalWrite(yellowPin, LOW); // Turn off yellow lights
    displayNumber(10);            // Turn off
  }

  // Buzzer logic
  if (buzzerState)
    digitalWrite(buzzerPin, HIGH); // Sound the buzzer
  else
    digitalWrite(buzzerPin, LOW); // Turn off the buzzer

  // Light logic
  if (!dimmer)
  {
    if (lightState)
    {
      digitalWrite(redPin, HIGH); // Turn lights to red
      digitalWrite(greenPin, LOW);
    }
    else
    {
      digitalWrite(greenPin, HIGH); // Turn lights to green
      digitalWrite(redPin, LOW);
    }
  }
  else
  {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
  }

  // Prints
  Serial.print("{pedState ");
  Serial.print(pedState);
  Serial.print("} {crossTimer ");
  Serial.print(crossTimer);
  Serial.print("} {lightState ");
  Serial.print(lightState);
  Serial.print("} {buzzerState ");
  Serial.print(buzzerState);
  Serial.print("} {crossState ");
  Serial.print(crossState);
  Serial.println("}");
}

long readUltrasonicDistance(int trigPin, int echoPin)
{
  // Clear the trig pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Set the tring pin high for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Read echo pin
  long duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in cm
  long distance = duration * 0.034 / 2; // Speed of sound is 0.034 cm/microsecond

  delay(30);
  return distance;
}

void displayNumber(int num)
{
  // Set the segments according to the number
  digitalWrite(segmentA, numbers[num][0]);
  digitalWrite(segmentB, numbers[num][1]);
  digitalWrite(segmentC, numbers[num][2]);
  digitalWrite(segmentD, numbers[num][3]);
  digitalWrite(segmentE, numbers[num][4]);
  digitalWrite(segmentF, numbers[num][5]);
  digitalWrite(segmentG, numbers[num][6]);
}