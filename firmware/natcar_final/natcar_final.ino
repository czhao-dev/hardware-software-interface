#include <Servo.h>

Servo steeringServo;

// Pin definitions
const byte PIN_CAMERA_AO = 22;
const byte PIN_CAMERA_CLK = 20;
const byte PIN_CAMERA_SI = 21;
const byte PIN_SERVO = 23;

// Reserved analog inputs for the archived inductor-based control experiments.
const byte PIN_LEFT_INDUCTOR = 19;
const byte PIN_CENTER_INDUCTOR = 18;
const byte PIN_RIGHT_INDUCTOR = 17;

const byte PIN_PWM_A = 4;
const byte PIN_PWM_B = 3;
const byte PIN_PWM_C = 9;
const byte PIN_PWM_D = 10;

// Camera settings
const int PIXEL_COUNT = 128;
const int CAMERA_CENTER_PIXEL = 58;
const int LINE_BRIGHTNESS_THRESHOLD = 200;
const int MIN_EDGE_STRENGTH = 25;
// Sanity bound on edge separation: rejects readings where the strongest
// rising/falling edges come from unrelated features rather than one line.
const int MAX_LINE_WIDTH_PIXELS = 40;
const int LOOP_DELAY_MS = 10;

// Speed tuning
const int TOP_SPEED_PWM = 100;
const int MIN_SPEED_PWM = 30;
const int STOP_SPEED_PWM = 0;
const int MAX_LOST_LINE_FRAMES = 25;
// Max PWM change per loop when recovering from a lost-line slowdown/stop.
const int MAX_PWM_STEP_PER_LOOP = 1;
const float TURN_SLOWDOWN_GAIN = 1.8;
const float SPEED_KP = 1.0;
const float SPEED_KD = 0.0;

// Steering tuning
const int SERVO_CENTER = 45;
const int SERVO_RANGE = 25;
const float STEERING_KP = 0.6;
const float STEERING_KD = 0.6;

struct LineDetection {
  bool detected;
  int position;
};

int pixels[PIXEL_COUNT];
int currentLinePosition = 0;
int previousLinePosition = 0;
float steeringAngle = SERVO_CENTER;
float leftMotorPwm = 0;
float rightMotorPwm = 0;
int lostLineFrames = 0;

void readCamera();
void startCameraFrame();
LineDetection detectLine();
void updateSteering(const LineDetection &line);
void updateMotorSpeed(const LineDetection &line);
float rampDown(float pwm, float minimumPwm);
float rampToward(float pwm, float target, float maxStep);
void writeMotorOutputs();
int clampPwm(float value);

void setup() {
  pinMode(PIN_PWM_A, OUTPUT);
  pinMode(PIN_PWM_B, OUTPUT);
  pinMode(PIN_PWM_C, OUTPUT);
  pinMode(PIN_PWM_D, OUTPUT);

  pinMode(PIN_CAMERA_CLK, OUTPUT);
  pinMode(PIN_CAMERA_SI, OUTPUT);
  pinMode(PIN_CAMERA_AO, INPUT);
  digitalWrite(PIN_CAMERA_CLK, LOW);
  digitalWrite(PIN_CAMERA_SI, LOW);

  pinMode(PIN_LEFT_INDUCTOR, INPUT);
  pinMode(PIN_CENTER_INDUCTOR, INPUT);
  pinMode(PIN_RIGHT_INDUCTOR, INPUT);

  writeMotorOutputs();
  steeringServo.attach(PIN_SERVO);
  steeringServo.write(SERVO_CENTER);
}

void loop() {
  readCamera();

  LineDetection line = detectLine();
  updateSteering(line);
  updateMotorSpeed(line);
  writeMotorOutputs();

  delay(LOOP_DELAY_MS);
}

void readCamera() {
  startCameraFrame();

  // Note: per the TSL1401 timing diagram, AO can lag the SI pulse by one
  // clock relative to this loop, which would shift pixels[] by a constant
  // pixel offset. CAMERA_CENTER_PIXEL was calibrated empirically against
  // this exact timing, so any change here must be re-calibrated on the car.
  for (int i = 0; i < PIXEL_COUNT; i++) {
    digitalWrite(PIN_CAMERA_CLK, HIGH);
    pixels[i] = analogRead(PIN_CAMERA_AO);
    digitalWrite(PIN_CAMERA_CLK, LOW);
  }
}

void startCameraFrame() {
  digitalWrite(PIN_CAMERA_SI, HIGH);
  digitalWrite(PIN_CAMERA_CLK, HIGH);
  digitalWrite(PIN_CAMERA_SI, LOW);
  digitalWrite(PIN_CAMERA_CLK, LOW);
}

LineDetection detectLine() {
  int headIndex = 0;
  int tailIndex = 0;
  int maxRisingEdge = -1024;
  int maxFallingEdge = 1024;

  for (int i = 0; i < PIXEL_COUNT - 1; i++) {
    const int diff = pixels[i + 1] - pixels[i];

    if (diff > maxRisingEdge) {
      maxRisingEdge = diff;
      headIndex = i;
    }

    if (diff < maxFallingEdge) {
      maxFallingEdge = diff;
      tailIndex = i;
    }
  }

  const int midpoint = (headIndex + tailIndex) / 2;
  const bool detected = headIndex < tailIndex
    && (tailIndex - headIndex) <= MAX_LINE_WIDTH_PIXELS
    && pixels[midpoint] > LINE_BRIGHTNESS_THRESHOLD
    && maxRisingEdge >= MIN_EDGE_STRENGTH
    && abs(maxFallingEdge) >= MIN_EDGE_STRENGTH;

  LineDetection result;
  result.detected = detected;
  result.position = currentLinePosition;

  if (!detected) {
    return result;
  }

  const int newPosition = midpoint - CAMERA_CENTER_PIXEL;

  // If the line was just reacquired after being lost, treat this frame as
  // having zero derivative instead of computing it against a stale position
  // from before the gap (which would spike the steering D-term).
  previousLinePosition = (lostLineFrames > 0) ? newPosition : currentLinePosition;
  currentLinePosition = newPosition;
  result.position = currentLinePosition;
  return result;
}

void updateSteering(const LineDetection &line) {
  if (line.detected) {
    const int positionDelta = line.position - previousLinePosition;
    steeringAngle = SERVO_CENTER
      - line.position * STEERING_KP
      - positionDelta * STEERING_KD;
  }

  const int minServoAngle = SERVO_CENTER - SERVO_RANGE;
  const int maxServoAngle = SERVO_CENTER + SERVO_RANGE;
  steeringAngle = constrain(steeringAngle, minServoAngle, maxServoAngle);

  steeringServo.write((int)steeringAngle);
}

void updateMotorSpeed(const LineDetection &line) {
  const float previousLeftPwm = leftMotorPwm;
  const float previousRightPwm = rightMotorPwm;

  if (!line.detected) {
    if (lostLineFrames < MAX_LOST_LINE_FRAMES) {
      lostLineFrames++;
    }

    if (lostLineFrames >= MAX_LOST_LINE_FRAMES) {
      leftMotorPwm = STOP_SPEED_PWM;
      rightMotorPwm = STOP_SPEED_PWM;
      return;
    }

    leftMotorPwm = rampDown(leftMotorPwm, MIN_SPEED_PWM);
    rightMotorPwm = rampDown(rightMotorPwm, MIN_SPEED_PWM);
    return;
  }

  const bool recovering = lostLineFrames > 0;
  lostLineFrames = 0;

  const int absLineError = abs(line.position);
  float targetPwm = TOP_SPEED_PWM - absLineError * TURN_SLOWDOWN_GAIN;
  targetPwm = max(targetPwm, (float)MIN_SPEED_PWM);

  leftMotorPwm = targetPwm * SPEED_KP + (targetPwm - previousLeftPwm) * SPEED_KD;
  rightMotorPwm = targetPwm * SPEED_KP + (targetPwm - previousRightPwm) * SPEED_KD;

  // After the line was lost (and possibly ramped down to a stop), ramp back
  // up to the target speed instead of snapping straight to it.
  if (recovering) {
    leftMotorPwm = rampToward(previousLeftPwm, leftMotorPwm, MAX_PWM_STEP_PER_LOOP);
    rightMotorPwm = rampToward(previousRightPwm, rightMotorPwm, MAX_PWM_STEP_PER_LOOP);
  }

  leftMotorPwm = max(leftMotorPwm, (float)MIN_SPEED_PWM);
  rightMotorPwm = max(rightMotorPwm, (float)MIN_SPEED_PWM);
}

float rampDown(float pwm, float minimumPwm) {
  if (pwm > minimumPwm) {
    return pwm - 1;
  }

  return minimumPwm;
}

float rampToward(float pwm, float target, float maxStep) {
  if (target > pwm + maxStep) {
    return pwm + maxStep;
  }

  if (target < pwm - maxStep) {
    return pwm - maxStep;
  }

  return target;
}

void writeMotorOutputs() {
  analogWrite(PIN_PWM_A, 0);
  analogWrite(PIN_PWM_B, clampPwm(leftMotorPwm));
  analogWrite(PIN_PWM_C, 0);
  analogWrite(PIN_PWM_D, clampPwm(rightMotorPwm));
}

int clampPwm(float value) {
  return constrain((int)value, 0, 255);
}
