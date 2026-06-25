#include <Servo.h>

Servo myservo;

/*defining the pins*/
int pin_AO = 22;
int pin_CLK = 20;
int pin_SI = 21;
int pin_servo = 23;
int pixels[128];
int pin_inductor1 = 19;
int pin_inductor2 = 18;
int pin_inductor3 = 17;
int PWM_A = 4;
int PWM_B = 3;
int PWM_C = 9;
int PWM_D = 10;

/*constants*/
const int THRESHHOLE = 1;
const int factor1 = 9;
const int factor2 = 9;
const int factor3 = 7;
const int servo_scale = 1;

//speeds
const int speed1 = 128;
const int speed2 = 96;
const int speed3 = 94;

//turn angles
const int turnAngle1 = 15;
const int turnAngle2 = 30;

//inductors
const int UPPER = 1;
const int LOWER = 10;

const int max_diff = 70;
int prev_inductor1 = 100;
int prev_inductor2 = 100;
int prev_inductor3 =100;
int prev_angle = 0;
int prev_speed = 0;

// PD control constants
double kp_a = 0;
double kp_s = 0;
double kd_a = 0.5;
double kd_s = 0.5;

void setup()
{
  pinMode(PWM_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(pin_CLK, OUTPUT);
  pinMode(pin_SI,OUTPUT);
  pinMode(pin_AO, INPUT);
  pinMode(pin_servo, OUTPUT);
  Serial.begin(9600);
  myservo.attach(pin_servo);
}

/* Initializes Camera */
void InitializeSI()
{
  digitalWrite(pin_SI, HIGH);
  digitalWrite(pin_CLK, HIGH);
  digitalWrite(pin_SI, LOW);
  digitalWrite(pin_CLK, LOW);
}

void loop()
{
  InitializeSI();
  
  // Read every pixel and store them in an array
  for(int i = 0; i < 128; i++)
  {
    digitalWrite(pin_CLK, HIGH);
    pixels[i] = analogRead(pin_AO);
    digitalWrite(pin_CLK, LOW);
  }
  
  // Inductor values
  int inductor1 = analogRead(pin_inductor1);
  int inductor2 = analogRead(pin_inductor2);
  int inductor3 = analogRead(pin_inductor3);
  
  // Positions of the line calculated by camera and AFE
  int positionCamera = PosCam(pixels);
  int positionAFE = PosAFE(inductor1, inductor2, inductor3);
  
  // Ask whether the line is detected by the camera or the AFE
  boolean lineDetected_Camera = lineDetectedCamera(positionCamera);
  boolean lineDetected_AFE = lineDetectedAFE(inductor1, inductor2, inductor3);
  
  // Previous angle and speed
  prev_angle = kp_a;
  kp_a = PosAFE(inductor1, inductor2, inductor3);
  prev_speed = kp_s;
  kp_s = PosCam(pixels);
  
  // Calculate the angle and the speed
  int a = CalculateAngle(positionCamera, kp_a, lineDetected_Camera, lineDetected_AFE);
  int s = CalculateSpeed(a, lineDetected_Camera, lineDetected_AFE);
  double PDSpeed = kp_s + kd_s * (kp_s - prev_speed);
  double PDAngle = kp_a + kd_a * (kp_a - prev_angle);
  
  // Printing out values 
  //------------------------------
  Serial.print(inductor1);
  Serial.print(" ");
  Serial.print(inductor2);
  Serial.print(" ");
  Serial.print(inductor3);
  Serial.print(" ");
  Serial.print("|");
  /*Serial.print(a);
  Serial.print("| cam pos: ");
  Serial.print(positionCamera);*/
 
 for(int i = 0; i < 128; i++)
 {
   Serial.print(pixels[i]/90);
   //Serial.print(" ");
 }
 
  Serial.print(" | ");
  Serial.print(positionCamera);
  //Serial.print(" | ");
  //Serial.print(s);
  Serial.print(" | ");
  Serial.print(a);
  //Serial.print(" | ");
  //Serial.print(PDAngle);
  Serial.print("\n");
  //------------------------------------
  
  // Controlling motor and servo
  MotorControl(PDSpeed);
  ServoControl(a);
  
  delay(25);
}

/* Input: array that stores the values of each pixel
   Output: Line position in that array 
           Returns -1 if line is not found */
int PosCam(int* pixels)
{
  int head, tail, midpoint;
  int maxDiff = 0;
  int minDiff = 0;
  
  for(int i = 1; i < 126; i++)
  {
      int diff = pixels[i+1]-pixels[i];
      if(diff >= maxDiff)
      {
        head = i;
        maxDiff = diff;
      }
      if (diff < minDiff)
      {
        tail = i;
        minDiff = diff;
      }
    }
    midpoint = (head + tail)/2;
    
    // If the maximum value of the pixels is less than the threshold,
    // then the line is not found
    if(pixels[midpoint] < 250) return -1;
    else return midpoint;
    
}

int PosAFE(int left, int center, int right)//Mi's angle //proportional coeffecients here!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 {//max turning angle 25 degrees, aka 20 to 70
   int angle = 45;
   int minangle = 20; int maxangle = 70;
   if((center > left) && (center > right))
   {
     if (center > 150) {angle = 45;}
     else if (left > right) 
       {
         angle = 45 - (250/ center);
         angle = (angle < minangle)? minangle:angle;
       }
     else 
       {
         angle = 45 + (250/ center);
         angle = (angle > maxangle)? maxangle:angle;
       }
   }
   else if (left > right+10) {angle = minangle;}
   else if (right > left+10) {angle = maxangle;}
   return angle;
 }

/* Returns true if the line is detected by the camera */
boolean lineDetectedCamera(int positionCamera)
{
  if(positionCamera > 0) return true;
  else return false;
}

/* Returns true if the line is detected by AFE */
boolean lineDetectedAFE(int left, int center, int right)
{
  if((left < LOWER) && (center < LOWER) && (right < LOWER)) return false;
  else return true;
}

/* Using PWM values to control the speed of the motor */
void MotorControl(int s)
{
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, s);
  analogWrite(PWM_C, 0);
  analogWrite(PWM_D, s);
}

/* Controlling servo's turn angle */
void ServoControl(int angle)
{
  myservo.write(angle);
}


/* Calculate the speed of the car given by the turn angle */
int CalculateSpeed(int angle, boolean lineDetectedCamera, boolean lineDetectedAFE)
{
  // If the line is not detected by both camera and AFE, then stop the car
  if(lineDetectedCamera==false && lineDetectedAFE==false)
  {
    return 0;
  }
  else
  {
    return 100-100*abs(45-angle);
  }
}

/* Calculate the turn angle given by the positions calculated by camera and AFE */
int CalculateAngle(int PosCam, int PosAFE, boolean lineDetectedCamera, boolean lineDetectedAFE)
{
  int angle = 45;
  if(lineDetectedCamera==true)
  {
      angle=(0.5)*(PosCam)+15;
  }
  else if((lineDetectedAFE==true) && (lineDetectedCamera==false))
  {
      angle=PosAFE;
  }
  
  return angle;
}
