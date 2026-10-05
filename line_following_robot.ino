// =====================================================
// 8 SENSOR LINE FOLLOWER
// Arduino UNO + L298N + RLS-08
// =====================================================


// =====================================================
// L298N
// =====================================================

const int ENA = 5;
const int IN1 = 3;
const int IN2 = 4;

const int IN3 = 7;
const int IN4 = 8;
const int ENB = 9;


// =====================================================
// RLS-08
// IR1 = LEFT
// IR8 = RIGHT
// =====================================================

const int IR1 = 2;
const int IR2 = 6;
const int IR3 = A0;
const int IR4 = A1;
const int IR5 = A2;
const int IR6 = A3;
const int IR7 = A4;
const int IR8 = A5;


// =====================================================
// SENSOR ARRAY
// =====================================================

int sensorPins[8] = {
  IR1, IR2, IR3, IR4,
  IR5, IR6, IR7, IR8
};


// =====================================================
// SENSOR WEIGHTS
// =====================================================

int weights[8] = {
  -35,   // IR1 LEFT
  -25,   // IR2
  -15,   // IR3
   -5,   // IR4
    5,   // IR5
   15,   // IR6
   25,   // IR7
   35    // IR8 RIGHT
};


// =====================================================
// RLS-08 LOGIC
//
// BLACK = HIGH
// WHITE = LOW
// =====================================================

const int LINE_DETECTED = HIGH;


// =====================================================
// PID
// =====================================================

float Kp = 6.0;
float Ki = 0.0;
float Kd = 7.0;

float error = 0;
float previousError = 0;

float integral = 0;
float derivative = 0;


// =====================================================
// SPEED
// =====================================================

int BASE_SPEED = 90;

int MIN_SPEED = 55;

int MAX_SPEED = 170;


// Normal PID maximum correction
int MAX_CORRECTION = 75;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  // Motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Sensor pins
  for (int i = 0; i < 8; i++) {
    pinMode(sensorPins[i], INPUT);
  }

  stopMotors();

  delay(1000);
}


// =====================================================
// CALCULATE ERROR
// =====================================================

float calculateError() {

  int weightedSum = 0;

  int activeSensors = 0;


  for (int i = 0; i < 8; i++) {

    int value = digitalRead(sensorPins[i]);

    if (value == LINE_DETECTED) {

      weightedSum += weights[i];

      activeSensors++;
    }
  }


  // Line detected
  if (activeSensors > 0) {

    return (float)weightedSum / activeSensors;
  }


  // Line lost
  return previousError;
}


// =====================================================
// FORWARD
// =====================================================

void moveForward(int leftSpeed, int rightSpeed) {

  // LEFT MOTOR FORWARD
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // RIGHT MOTOR FORWARD
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}


// =====================================================
// STOP
// =====================================================

void stopMotors() {

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  // =================================================
  // READ SENSORS
  // =================================================

  int s1 = digitalRead(IR1);
  int s2 = digitalRead(IR2);
  int s3 = digitalRead(IR3);
  int s4 = digitalRead(IR4);
  int s5 = digitalRead(IR5);
  int s6 = digitalRead(IR6);
  int s7 = digitalRead(IR7);
  int s8 = digitalRead(IR8);


  // =================================================
  // VERY STRONG LEFT TURN
  //
  // IR1 sees the line strongly on the left.
  // =================================================

  if (s1 == LINE_DETECTED) {

    Serial.println("VERY SHARP LEFT");


    // Reset PID memory
    integral = 0;
    previousError = -35;


    // Pivot strongly LEFT
    moveForward(20, 170);


    delay(15);

    return;
  }


  // =================================================
  // VERY STRONG RIGHT TURN
  //
  // IR8 sees the line strongly on the right.
  // =================================================

  if (s8 == LINE_DETECTED) {

    Serial.println("VERY SHARP RIGHT");


    // Reset PID memory
    integral = 0;
    previousError = 35;


    // Pivot strongly RIGHT
    moveForward(170, 20);


    delay(15);

    return;
  }


  // =================================================
  // STRONG LEFT CURVE
  //
  // IR2 detects the line.
  // =================================================

  if (s2 == LINE_DETECTED) {

    Serial.println("STRONG LEFT");


    integral = 0;

    previousError = -25;


    // Strong left correction
    moveForward(40, 155);


    delay(10);

    return;
  }


  // =================================================
  // STRONG RIGHT CURVE
  //
  // IR7 detects the line.
  // =================================================

  if (s7 == LINE_DETECTED) {

    Serial.println("STRONG RIGHT");


    integral = 0;

    previousError = 25;


    // Strong right correction
    moveForward(155, 40);


    delay(10);

    return;
  }


  // =================================================
  // NORMAL PID
  // =================================================

  error = calculateError();


  // =================================================
  // INTEGRAL
  // =================================================

  integral += error;

  integral = constrain(
    integral,
    -100,
    100
  );


  // =================================================
  // DERIVATIVE
  // =================================================

  derivative = error - previousError;


  // =================================================
  // PID
  // =================================================

  float correction =
      (Kp * error)
      +
      (Ki * integral)
      +
      (Kd * derivative);


  // =================================================
  // LIMIT CORRECTION
  // =================================================

  correction = constrain(
    correction,
    -MAX_CORRECTION,
    MAX_CORRECTION
  );


  previousError = error;


  // =================================================
  // MOTOR SPEED
  // =================================================

  int leftSpeed =
      BASE_SPEED + correction;

  int rightSpeed =
      BASE_SPEED - correction;


  // =================================================
  // LIMIT SPEED
  // =================================================

  leftSpeed = constrain(
    leftSpeed,
    MIN_SPEED,
    MAX_SPEED
  );

  rightSpeed = constrain(
    rightSpeed,
    MIN_SPEED,
    MAX_SPEED
  );


  // =================================================
  // MOVE
  // =================================================

  moveForward(
    leftSpeed,
    rightSpeed
  );


  // =================================================
  // SERIAL MONITOR
  // =================================================

  Serial.print("Error: ");
  Serial.print(error);

  Serial.print(" | Correction: ");
  Serial.print(correction);

  Serial.print(" | L: ");
  Serial.print(leftSpeed);

  Serial.print(" | R: ");
  Serial.println(rightSpeed);


  delay(8);
}