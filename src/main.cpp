#include <Arduino.h>
#include <Servo.h>

// Function declarations
long readUltrasonicDistance(int, int);

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

// Create objects
Servo gate;

// Constants
const double minDistance = 100;
const int crossTime = 10 * 1000; // 10 seconds
const int gateUp = 70;
const int gateDown = 35;

// States
float crossStartTime = 0;
bool buttonQueue = false;
bool warningState = false;

void setup()
{
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buttonPin, INPUT);
  gate.attach(servoPin);
}

void loop()
{
  // Read the button state and set queue
  if (digitalRead(buttonPin) == 0)
  {
    Serial.print("{Button PRESS} ");
    buttonQueue = true;
  }

  // Timer for crossing
  long crossTimer = millis() - crossStartTime;

  // Read the distance
  long distance = readUltrasonicDistance(trigPin, echoPin);
  Serial.print("{Distance ");
  Serial.print(distance);
  Serial.println("}");

  // Set timer for cross if safe
  if (buttonQueue && distance >= minDistance)
  {
    crossStartTime = millis();
    buttonQueue = false;
  }
  // Set warning if queue, or timer is active
  else if (buttonQueue || crossTimer < crossTime)
    warningState = true;

  // Check warning state
  if (warningState)
  {
    digitalWrite(redPin, HIGH);    // Turn lights to red
    digitalWrite(buzzerPin, HIGH); // Sound the buzzer
  }
  else
  {
    digitalWrite(redPin, LOW);    // Turn red light off
    digitalWrite(buzzerPin, LOW); // Turn buzzer off
  }

  // Check if timer is active
  if (crossTimer < crossTime)
  {
    gate.write(gateDown);          // Set the servo into down position
    digitalWrite(yellowPin, HIGH); // Set flashing yellow lights
  }
  else
  {
    gate.write(gateUp);           // Move the gate up
    digitalWrite(yellowPin, LOW); // Turn off the lights
  }

  // Display countdown to segment
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
  return distance;
}