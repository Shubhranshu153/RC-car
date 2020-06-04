#include <WiFi.h> //Header file for WiFi module 
#include <WiFiUDP.h> //Header File for using UDP communication

// variable declaration for motor Control
#define right_motor_channel 0 // Variable for right motor channel used in LEDC func
#define right_motor_enable 33 //Pin 33 is used to enable the right motor channel  
#define right_motor_dir 16 // Pin 16 is used to set the direction of right motor  
#define left_motor_channel 1 // Variable for left motor channel used in LEDC func
#define left_motor_enable 32//Pin 32 is used to enable the left motor channel  
#define left_motor_dir 17 // Pin 12 is used to set the direction of left motor

const char* ssid = "Mechatronics"; //Wifi Name
const char* password = "YayFunFun";//Wifi passoword
volatile int flag1 = 0;//For Go command receive check

WiFiUDP UDP; //Instance UDPTesTserver to receive the RC car speed and direction 
unsigned int new_UDPPort = 2900; //port for communication of speed and direction from the controller
IPAddress myIPaddress(192, 168, 1, 163);    // my assigned ip


const int packetSize = 2; // Size of speed and direction 1 byte each
char packetBuffer1[packetSize];  //buffer to receive values of speed and direction

/***************************** *****variables for right motor duty cycle
, left motor duty cycle,speed adc value and dircetion ADC values */ 
int right_motor;
int left_motor;
int speed_adc;
int direction_adc;


int freq =200;         //frequency of pwm set frequency to 200 ledc func
int resolution = 7;     //7 bit resolution for ledc func
//*************************************


int reciever() {  // Receive speed and direction ADC values              
  int cb = UDP.parsePacket(); //Function gives 0 until a packet is received
  if (cb) { // if value received
    UDP.read(packetBuffer1, packetSize); // read and store the value in packetBuffer1
    speed_adc = packetBuffer1[0] ; // speed adc value is sent in 1st byte
    direction_adc = packetBuffer1[1]; //direction adc value is sent in 2nd byte
    //Serial.println(right_motor); serial communcations commented out
    //Serial.println(left_motor);
    digitalWrite(2,!digitalRead(2));// Blue LED toggle
  }
 
  return (cb);// returns value to run the motor control 
}

void setup() {
  //Serial.begin(115200); //for no serial communication in final code

  //************************LEDc setup code

  ledcSetup(right_motor_channel, freq, resolution); // right motor LEDC setup
  ledcSetup(left_motor_channel, freq, resolution); //left motor LEDC setup
  ledcAttachPin(right_motor_enable, right_motor_channel); // Pin attached for right motor 
  ledcAttachPin(left_motor_enable, left_motor_channel); //pin attached for left motor

  //Motor controls pins and LED is set to ooutput
  pinMode(right_motor_enable, OUTPUT);
  pinMode(right_motor_dir, OUTPUT);
  pinMode(left_motor_enable, OUTPUT);
  pinMode(left_motor_dir, OUTPUT);
  pinMode(2, OUTPUT);
 
  //**************** Wifi setup. Serial communication are commented out
 
  WiFi.begin(ssid, password); //Setting password and ssid to connet to WiFi
  WiFi.config(myIPaddress, IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));//Setting up the addresses
 // UDPTestServer.begin(UDPPort); //Open UDP channel for GO!
  UDP.begin(new_UDPPort); //Open UDP channel for ADC values
  while (WiFi.status() != WL_CONNECTED)//wait till Connect to Wifi
  { delay(500);
    Serial.print(".");
  }
  //Serial.println("WiFi Connected");// all serial communication disabled
  digitalWrite(2, HIGH);// when connected to wifi and GO receive glow blue light
}

void loop()
{
  
int k1=2; //constant for direction control
int k2=2; // constant for speed control

// motor control circuit uses inverter so one control pin is used
  
  if (reciever()) {     // check if packet is received
    if (speed_adc > 48) { // speed adc is mapped 0 to 100 if > 48 its forward and < 45 reverse between dead zone
      speed_adc=(speed_adc-48); // speed adc centering
      if(direction_adc>52)// direction_adc >52 is left turn using differential control
      {
      right_motor = (speed_adc*k2 +(direction_adc-52)/k1);
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, HIGH);
      
      left_motor = (speed_adc*k2 - (direction_adc-52)/k1);
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, HIGH); 
    } 
    else if(direction_adc<45)// direction_adc <45 is right turn using differential control
      {
      right_motor = (speed_adc*k2 - (-direction_adc+45)/k1);
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, HIGH);
      
      left_motor = (speed_adc*k2 + (-direction_adc+45)/k1);
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, HIGH);  
    } 
    else // go straight forward
    {
      right_motor = speed_adc*k2;
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, HIGH);
      
      left_motor = speed_adc*k2;
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, HIGH);  
    }
    }
    else if(speed_adc<47) // go reverse
    {
     speed_adc = (45 - speed_adc); // normalizing the speed adc
     if(direction_adc>52)  // direction_adc >52 is left turn using differential control
      {
      right_motor = (speed_adc*k2 + (direction_adc-52)/k1);
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, LOW);
      
      left_motor = (speed_adc*k2 - (direction_adc-52)/k1);
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, LOW); 
    } 
    else if(direction_adc<45)// direction_adc <45 is right turn using differential control
      {
      right_motor =(speed_adc*k2 - (-direction_adc+45)/k1);
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, LOW);
      
      left_motor = (speed_adc*k2 + (-direction_adc+45)/k1);
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, LOW);  
    }
    else // go straight in reverse direction
    {
      right_motor = speed_adc*k2;
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, LOW);
      
      left_motor = speed_adc*k2;
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, LOW);  
    }
    }
    else // if no value received stop
    {
      right_motor =0;
      ledcWrite(right_motor_channel, right_motor);
      digitalWrite(right_motor_dir, LOW);
      
      left_motor = 0;
      ledcWrite(left_motor_channel, left_motor);
      digitalWrite(left_motor_dir, LOW);  
       
  }
  delay(30); // to sync with send and receive
}

}
