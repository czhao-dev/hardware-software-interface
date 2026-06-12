#include <Servo.h>
Servo myservo;

//Pin defination
int pin_AO = 22;
int pin_CLK = 20;
int pin_SI = 21;
int pin_servo = 23;
int left_inductor_pin = 19;
int center_inductor_pin = 18;
int right_inductor_pin = 17;
int PWM_A = 4;
int PWM_B = 3;
int PWM_C = 9;
int PWM_D = 10;



//Speed Parameters
const int topSpeed = 120;



//Steering Parameters
float skp = 1;//speed Kp
float skd = 0;//speed Kd

float kp = 0.9;//angle Kp 
float kd = 0;//angle Kd
const int midPosition = 45;
const int range = 25;

float angle;
float prev_angle = 45;

int left;
int right;
int prevl;
int prevr;

//Cam setting
int CAM_state;
int pixels[128];
int CAMmid = 58;
int CAM_position;
int PRE_CAM_positionl;

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
  left = 0;
  right = 0;
}


void loop()
{
  initialCam();
  readCam();
  CamPos();
  ServoControl();
  MotorControl();
  delay(10);
  printCAM();
  //printControl();
  Serial.print("\n");
}

void printControl()
{
  
  Serial.print("camstate: | ");
  Serial.print(CAM_state);
}


void printCAM()
{
  Serial.print(" cam | ");
  Serial.print(CAM_state);
  Serial.print("|");
  Serial.print(CAM_position);
 
  //Serial.print(PRE_CAM_positionl);
}


void MotorControl()
{ 
  prevl = left;
  prevr = right;
  left = topSpeed - abs(CAM_position)*2  ;//deceleration here!!!!!!
  left = left*skp + skd* (left - prevl);
     //left = 0;
  right = left;
  Serial.print(" L |");
  Serial.print(left);
  Serial.print(" R |");
  Serial.print(right);
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, left);
  analogWrite(PWM_C, 0);
  analogWrite(PWM_D, right);
}

void ServoControl()
{  
  prev_angle = angle;
  angle = -CAM_position/1.5+45;//here!!!!!!!!!!!!!!!!!!
  angle = kp*angle + kd * (angle - prev_angle);
  angle = max(midPosition-range, angle);
  angle = min(midPosition+range, angle);
  Serial.print(" angle |  ");
  Serial.print(angle);
  myservo.write(int(angle));
}

void initialCam()
{
  digitalWrite(pin_SI, HIGH);
  digitalWrite(pin_CLK, HIGH);
  digitalWrite(pin_SI, LOW);
  digitalWrite(pin_CLK, LOW);
}


void readCam()
{
  initialCam();
  for(int i = 0; i < 128; i++)
  {
    digitalWrite(pin_CLK, HIGH);
    pixels[i] = analogRead(pin_AO);
    digitalWrite(pin_CLK, LOW);
    Serial.print(pixels[i]/70);
    //Serial.print(" ");   
  }
  Serial.print("\n");
}

void CamPos()
{

  int head, tail, midPoint;
  int maxDiff = 0;
  int minDiff = 0;
  int diff;
  

  
  for(int i = 0; i < 127; i++)
  {
    diff = pixels[i+ 1] - pixels[i];

    if (diff < minDiff)
    {
      tail = i;
      minDiff = diff;
    }
    
    if (diff > maxDiff)
    {
      head = i;
      maxDiff = diff;
    }
  }
    midPoint = (head + tail)/2;
    
    if(pixels[midPoint] > 200)
    {
      CAM_state = 1;
      PRE_CAM_positionl = CAM_position;
      CAM_position = midPoint-CAMmid;
    }
    else
    {
      CAM_state = 0;
    }
}

