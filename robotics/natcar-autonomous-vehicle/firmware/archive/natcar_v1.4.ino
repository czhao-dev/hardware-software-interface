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
//const int PWM_scale = 1;
const int servo_scale = 1;

//speeds
const int speed1 = 128;
const int speed2 = 96;
const int speed3 = 64;

//turn angles
const int turnAngle1 = 15;
const int turnAngle2 = 30;

//inductors
const int UPPER = 1;
const int LOWER = 1;

const int max_diff = 70;
int prev_inductor1 = 100;
int prev_inductor2 = 100;
int prev_inductor3 =100;
int prev_angle = 0;
int a2 = 0;

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

void InitializeSI()
{
  digitalWrite(pin_SI, HIGH);
  digitalWrite(pin_CLK, HIGH);
  digitalWrite(pin_SI, LOW);
  digitalWrite(pin_CLK, LOW);
}

void loop()
{
  //InitializeSI();
  //delay(200);
  InitializeSI();
  
  for(int i = 0; i < 128; i++)
  {
    digitalWrite(pin_CLK, HIGH);
    pixels[i] = analogRead(pin_AO);
    digitalWrite(pin_CLK, LOW);
  }
  
  int inductor1 = analogRead(pin_inductor1);
  int inductor2 = analogRead(pin_inductor2);
  int inductor3 = analogRead(pin_inductor3);
  
  int positionCamera = PosCam(pixels);
  
  /*if (abs(prev_inductor1-inductor1) > max_diff) inductor1=prev_inductor1;
  else prev_inductor1=inductor1;
  
  if (abs(prev_inductor2-inductor2) > max_diff) inductor2=prev_inductor2;
  else prev_inductor2=inductor2;
  
  if (abs(prev_inductor3-inductor3) > max_diff) inductor3=prev_inductor3;
  else prev_inductor3=inductor3;*/
  
  int positionAFE = PosAFE(inductor1, inductor2, inductor3);
  
  boolean lineDetected_Camera = lineDetectedCamera(positionCamera);
  boolean lineDetected_AFE = lineDetectedAFE(inductor1, inductor2, inductor3);
  int s = CalculateSpeed(positionCamera, 0, lineDetected_Camera, false)/3;
  int a = CalculateAngle(positionCamera, positionAFE, lineDetected_Camera, false);
  //int angle = (0.5)*(positionCamera)+15;
  
  /*Serial.print(inductor1);
  Serial.print(" ");
  Serial.print(inductor2);
  Serial.print(" ");
  Serial.print(inductor3);
  Serial.print(" ");
  Serial.print("|");
  Serial.print(a);
  Serial.print("| cam pos: ");
  Serial.print(positionCamera);*/
 
 for(int i = 0; i < 128; i++)
 {
   Serial.print(pixels[i]/90);
   //Serial.print(" ");
 }
 
  Serial.print(" | ");
  Serial.print(positionCamera);
  Serial.print(" | ");
  Serial.print(s);
  Serial.print(" | ");
  Serial.print(a);
  
  Serial.print("\n");
  MotorControl(s);
  ServoControl(a);
  
  delay(25);
}

int PosCam(int* pixels)
{
  int head, tail, midpoint;
  int maxDiff = 0;
  int minDiff = 0;
  int a1 = 0;
  int a2 = 0;
  int b1 = 0;
  int b2 = 0;
  for(int i = 1; i < 126; i++)
  {
    int diff = pixels[i+1]-pixels[i];
    if(diff >= maxDiff)
    {
      head = i;
      maxDiff = diff;
      a1 = pixels[i];
      a2 = pixels[i+1];
    }
    if (diff < minDiff)
    {
      tail = i;
      minDiff = diff;
      b1 = pixels[i];
      b2 = pixels[i+1];
    }
  }
  midpoint = (head + tail)/2;
  
  //if(((a2-a1)/a1) >= 1 && ((b1-b2)/b2) >= 1) return midpoint; 
  //else return -1;
  if(pixels[midpoint] < 250) return -1;
  else return midpoint;
}

boolean lineDetectedCamera(int positionCamera)
{
  if(positionCamera > 0) return true;
  else return false;
}

int PosAFE(int left, int center, int right)
{
  /*
  int pos = 0;
  if((center > left) && (center > right))
  {
    pos = (410.16-center)/40.2;
  }
  else if((left > center) && (left > right))
  {
    pos = ((415 - left)/52)-10;
  }
  else if((right > center) && (right > right))
  {
    pos = ((391 - right)/40)+10;
  }
  else return pos;
  */
  
  if((center > left) && (center > right)) return 0;
  else if((left > center) && (left > right))
  {  
    if(left < 100) return -2;
    else return -1;
  }
  else if((right > center) && (right > left)) 
  {  if(left < 100) return 2;
     else return 1;
  }
  else return 0;
}

boolean lineDetectedAFE(int left, int center, int right)
{
  if((left < LOWER) && (center < LOWER) && (right < LOWER)) return false;
  else return true;
}

void MotorControl(int s)
{
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, s);
  analogWrite(PWM_C, 0);
  analogWrite(PWM_D, s);
}

void ServoControl(int angle)
{
  myservo.write(angle);
}

int CalculateSpeed(int PosCam, int PosAFE, boolean lineDetectedCamera, boolean lineDetectedAFE)
{
  if(lineDetectedCamera==false && lineDetectedAFE==false)
  {
    return 0;
  }
  else if(!(PosCam >= 49 && PosCam <= 79))
  {
    if(PosAFE == 0) return speed2;
    else return speed3;
  }
  else return speed1;
  
}

int CalculateAngle(int PosCam, int PosAFE, boolean lineDetectedCamera, boolean lineDetectedAFE)
{
  int angle = 45;
  if(lineDetectedCamera==true)
  {
      angle=(0.5)*(PosCam)+15;
  }
  else if((lineDetectedAFE==true) && (lineDetectedCamera==false))
  {
    switch(PosAFE)
    {
      case -2:
      angle = 45 - turnAngle2;
      break;
      case -1:
      angle = 45 - turnAngle1;
      break;
      case 1:
      angle = 45 + turnAngle1;
      break;
      case 2:
      angle = 45 + turnAngle2;
      break;
    }
  }
  
  return angle;
}
