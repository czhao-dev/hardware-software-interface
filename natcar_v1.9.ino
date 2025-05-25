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
int delayt = 10;


//Speed Parameters
const int topSpeed = 100;

//Steering Parameters
float kp_s = 1;//speed Kp
float kd_s = 0;//speed Kd

float kp_a = 0.6;//angle Kp 
float kd_a = 0.6;//angle Kd
const int midPosition = 45;
const int range = 25;

float angle;
float prev_angle = 45;

float left_PWM;
float right_PWM;
float prev_left_PWM;
float prev_right_PWM;

//Cam setting
int CAM_state;
int pixels[128];
int CAMmid = 58;
int CAM_position;
int PRE_CAM_position;

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
  left_PWM = 0;
  right_PWM = 0;
}

void loop()
{
  //initializeCam();
  readCam();
  CamPos();
  ServoControl();
  MotorControl();
  delay(delayt);
  
  //printCAM();
  //printControl();
  //Serial.print("\n");
}

/*void printControl()
{
  
  Serial.print("camstate: | ");
  Serial.print(CAM_state);
}*/


/*void printCAM()
{
  Serial.print(" cam | ");
  Serial.print(CAM_state);
  Serial.print("|");
  Serial.print(CAM_position);
 
  //Serial.print(PRE_CAM_positionl);
}*/

void MotorControl()
{  
  prev_left_PWM = left_PWM;
  prev_right_PWM = right_PWM;
  if(CAM_state == 0)
  {
    if(left_PWM >30)
    left_PWM--;
    else
    left_PWM = 30;
    
    if(right_PWM > 30)
    right_PWM --;
    else
    right_PWM = 30;
  }
  else
  {
    left_PWM = topSpeed - abs(CAM_position)*1.8;//deceleration here!!!!!! 
    left_PWM = left_PWM*kp_s + kd_s*(left_PWM - prev_left_PWM); 
    
    if(left_PWM < 30) left_PWM = 30;
    
    right_PWM = left_PWM;
  }
  //Serial.print(" L |");
  //Serial.print(left);
  //Serial.print(" R |");
  //Serial.print(right);
  
  
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, left_PWM);
  analogWrite(PWM_C, 0);
  analogWrite(PWM_D, right_PWM);
}

void ServoControl()
{
  prev_angle = angle;
  if(CAM_state == 0)
  {
    angle = angle;  
  }
  else angle = 45 - CAM_position * kp_a - (CAM_position - PRE_CAM_position)*kd_a;
  //angle = kp_a*angle + kd_a * (angle - prev_angle);
  angle = max(midPosition-range, angle);
  angle = min(midPosition+range, angle);
  
  //Serial.print(" angle |  ");
  //Serial.print(angle);
  
  myservo.write(int(angle));
}

void initializeCam()
{
  digitalWrite(pin_SI, HIGH);
  digitalWrite(pin_CLK, HIGH);
  digitalWrite(pin_SI, LOW);
  digitalWrite(pin_CLK, LOW);
}


void readCam()
{
  initializeCam();
  for(int i = 0; i < 128; i++)
  {
    digitalWrite(pin_CLK, HIGH);
    pixels[i] = analogRead(pin_AO);
    digitalWrite(pin_CLK, LOW);
    //Serial.print(pixels[i]/70);
    //Serial.print(" ");   
  }
  //Serial.print("\n");
}

void CamPos()
{
  int head, tail, midPoint, diff;
  int maxDiff = 0;
  int minDiff = 0;
  
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
      PRE_CAM_position = CAM_position;
      CAM_position = midPoint-CAMmid;
    }

    
}

