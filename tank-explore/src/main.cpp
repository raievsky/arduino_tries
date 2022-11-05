#include <Arduino.h>

// Pins configuration
constexpr int trigPin = 12; // Pin Trig attach to 12
constexpr int echoPin = 13; // Pin Echo attach to 13
constexpr int sonarServoPin = 10; // Pin to control sonar's servo

constexpr int leftDriveCtrlPin = 2;
constexpr int rightDriveCtrlPin = 4;
constexpr int leftDrivePWM = 9;
constexpr int rightDrivePWM = 5;

// Drive corrections
constexpr float backPWMCorrection = 0.95;
constexpr float forwardPWMCorrection = 0.925;

// Distances
constexpr int safeDistmm = 150;
int maxScanDist = 0;
int scanAngle = -1;
int maxScanAngle = -1;
int spaceLeft = 0;
int spaceRight = 0;

float speed = 0.7f;

// Functions
int32_t sonarDistance();
void turnSonar(int angle);
void stopTank();
void moveTank(bool direction, float speed);
void forwardTank(float speed);
void backwardTank(float speed);
void fullLeftTrack();
void fullRightTrack();

enum State {
    start,
    explore,
    stop,
    scanning,
    turnLeft,
    turnRight,
    back
};

State state = start;

void setup()
{
    // Serial Port begin
    Serial.begin(9600); // Set the baud rate to 9600

    // Define input and output
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(sonarServoPin, OUTPUT);
    pinMode(leftDriveCtrlPin, OUTPUT);
    pinMode(rightDriveCtrlPin, OUTPUT);
    pinMode(leftDrivePWM, OUTPUT);
    pinMode(rightDrivePWM, OUTPUT);
}

void loop()
{
    auto distance = sonarDistance();

    switch (state)
    {
    case explore:
        if (distance < safeDistmm)
        {
            stopTank();
            state = stop;
        }
        break;

    case back:
        if (distance > 1.5*safeDistmm)
        {
            stopTank();
            state = stop;
        }

        break;

    case stop:
        if (distance < 60)
        {
            backwardTank(0.75);
            state = back;
        }
        else {
            state = scanning;
        }

        break;

    case start:
        stopTank();
        turnSonar(90);

        if (distance > 50 && distance < 120)
        {
            moveTank(true, speed);
            state = explore;
        }
        break;

    case scanning:
        
        if (scanAngle == -1){
            // Go backward to get some space
            moveTank(false, 0.7f);
            delay(300);
            stopTank();

            // move sonar to initial scanning position
            turnSonar(0);
            scanAngle = 0;
        }
        else if (scanAngle <= 180)
        {
            // Get fresh distance
            distance = sonarDistance();

            if (distance > maxScanDist) // Update max distance
            {
                maxScanDist = distance;
                maxScanAngle = scanAngle;
            }

            if (scanAngle < 90)
            {
                spaceRight += distance;
            }
            else {
                spaceLeft += distance;
            }
            
            // move sonar to next scanning position
            turnSonar(scanAngle);
            scanAngle += 15;
        } else
        {   
            // Done scanning
            turnSonar(90);

            // if (maxScanAngle < 90)
            // {
            //     state = turnLeft;
            // }
            // else {
            //     state = turnRight;
            // }

            if (spaceLeft > spaceRight)
            {
                Serial.print(spaceLeft);
                Serial.print(" > ");
                Serial.print(spaceRight);
                Serial.print(" left ");
                state = turnLeft;
                // fullLeftTrack();
            }
            else
            {
                Serial.print(spaceRight);
                Serial.print(" > ");
                Serial.print(spaceLeft);
                Serial.print(" right ");
                state = turnRight;
                // fullRightTrack();
            }

            spaceLeft = 0;
            spaceRight = 0;
            scanAngle = -1;
        }

        break;

    case turnLeft:
    {
        // Give some time to the robot to turn
        fullRightTrack();
        // const int delayms = 800 * ((90 - maxScanAngle) / 90.0f);
        const int delayms = 300;
        Serial.print(delayms);
        delay(delayms);
        state = explore;
        maxScanDist = 0;
        maxScanAngle = -1;
        moveTank(true, 0.8f);
    }
        break;

    case turnRight:
    {
        // Give some time to the robot to turn
        fullLeftTrack();
        // const int delayms = 800 * (maxScanAngle / 180.0f);
        const int delayms = 300;
        Serial.print(delayms);
        delay(delayms);
        state = explore;
        maxScanDist = 0;
        maxScanAngle = -1;
        moveTank(true, 0.8f);
    }
        break;

    default:
        stopTank();
        break;
    }

    // if (distance < 60)
    // {
    // backwardTank(0.75);
    // }
    // else if (distance < 150)
    // {
    // stopTank();
    // }
    // else{
    // moveTank(true, 0.8f);
    // }

    // Serial port print it
    // Serial.print(distance);
    // Serial.print(" mm");
    // Serial.println();
    delay(50);
}

void stopTank()
{
  digitalWrite(rightDriveCtrlPin, LOW);
  analogWrite(rightDrivePWM, 0);
  digitalWrite(leftDriveCtrlPin, LOW);
  analogWrite(leftDrivePWM, 0);
}

void moveTank(bool direction, float speed){
  if (speed > 1.0f)
  {
    speed = 1.0f;
  }
  else if (speed < 0.0f)
  {
    speed = 0.0f;
  }

  const int pwm = 255*speed;
  const int backCorrectedPWM = 255*speed*backPWMCorrection;
  const int forwardCorrectedPWM = 255*speed*forwardPWMCorrection;

  if (direction)
  {
    digitalWrite(rightDriveCtrlPin, HIGH);
    digitalWrite(leftDriveCtrlPin, HIGH);
    analogWrite(rightDrivePWM, forwardCorrectedPWM);
  } else {
    digitalWrite(rightDriveCtrlPin, LOW);
    digitalWrite(leftDriveCtrlPin, LOW);
    analogWrite(rightDrivePWM, backCorrectedPWM);
  }
  
  analogWrite(leftDrivePWM, pwm);

}

void forwardTank(float speed)
{
  moveTank(true, speed);
}

void backwardTank(float speed)
{
  moveTank(false, speed);
}

void fullLeftTrack(){
    digitalWrite(leftDriveCtrlPin, HIGH);
    analogWrite(leftDrivePWM, 250);
}

void fullRightTrack(){
    digitalWrite(rightDriveCtrlPin, HIGH);
    analogWrite(rightDrivePWM, 250);
}

int32_t sonarDistance() {

  // Send a pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); // At least give 10us high level trigger
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // The time in high level equals the time gap between the transmission and the return of the ultrasonic sound
  const auto duration = pulseIn(echoPin, HIGH);

  // Translate into distance
  const int32_t distance = round((duration / 2.) / 2.91);
  return distance;
}

void turnSonar(int angle)
{
    const int turnSteps = 10;
    const int pulseWidth = angle * 11 + 500; // Calculate the value of pulse width;
    for (int i = 0; i < turnSteps; i++)
    {
        digitalWrite(sonarServoPin, HIGH);
        delayMicroseconds(pulseWidth); // The time in high level represents the pulse width
        digitalWrite(sonarServoPin, LOW);
        delay((20 - pulseWidth / 1000));
    }
}
