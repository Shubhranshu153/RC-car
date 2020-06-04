
#define rpot 32 //Speed ADC control
#define lpot 33 // Direction ADC control
#include <WiFi.h>//Header file for WiFi module 
#include <WiFiUDP.h>//Header File for using UDP communication

const char* ssid = "Mechatronics";//Wifi Name
const char* password = "YayFunFun";//Wifi passoword
WiFiUDP udp,UDPTestServer;//Instance udp to send the RC car speed and direction 
const int UDP_PACKET_SIZE = 2; //2 bytes sent 1 for speed and 1 for diretion
char udpBuffer[UDP_PACKET_SIZE];// sent packet buffer
IPAddress ipTarget(192, 168, 1, 163);    //Target IP ti send
unsigned int UDPPort = 2390;        // port for communication of the GO command
IPAddress myIPaddress(192, 168, 1, 179);    // my assigned ip
int flag1=0;

const int UDP_PACKET_SIZE1 = 4; // Size of the packetBuffer Array to receive GO! along with null
char packetBuffer[UDP_PACKET_SIZE1];      // buffer to receive values of GO
//**********************************variables for direction ADC and Speed ADc
int direction_adc; 
int speed_adc;

//*************************************

void handleUDPServer() {  //func to receive GO! command              
  int cb = UDPTestServer.parsePacket();//Function gives 0 until a packet is received
  if (cb) { // if value received
    UDPTestServer.read(packetBuffer, UDP_PACKET_SIZE1); // read the communication
   // printf("%s\n", packetBuffer); //commented out as in final code we dont want serial communcation for sake of fast communication.
    if (strcmp(packetBuffer, "GO!") == 0) // if Go received set value to 1
    { flag1 = 1;
    }
  }
}
void setup() {

  pinMode(rpot, INPUT); //ADC input for speed
  pinMode(lpot, INPUT); //ADC input for direction
  pinMode(2, OUTPUT); // Blue LED glow while sending
  
  //serial communication is stopped 
 // Serial.begin(115200);
  //Serial.print("Connecting to ");
  //Serial.println(ssid);
  
  WiFi.config(IPAddress(192, 168, 1, 179), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0)); //sender wifi address in the server
  WiFi.begin(ssid, password);//wifi and password setup
  udp.begin(2900); //UDP channel for sending the values
  UDPTestServer.begin(UDPPort);

  while (WiFi.status() != WL_CONNECTED) {// wait till wifi connected
    delay(500);
    //Serial.print(".");
  }
   while (flag1 == 0)//wait till GO! received 
  { handleUDPServer() ;
  }
  digitalWrite(2,HIGH); // glow in built blue LED
}
void fncUdpSend(int q, int w) // function to send speed adc and direction adc
{
  udpBuffer[0] = q; // send rpot
  udpBuffer[1] = w; // send lpot
  
  udp.beginPacket(ipTarget, 2900); // setup UDC channel to UDPport 2900
  udp.printf("%s", udpBuffer); // send the packet through 2900 port
  udp.endPacket(); //end sending packet
  digitalWrite(2,!digitalRead(2));//toggle onboard blue LED

}

void loop() {
  direction_adc = map(analogRead(lpot), 0, 4095, 1, 100);// direction ADC value is mapped from 1 to 100
  speed_adc = map(analogRead(rpot), 0, 4095, 1, 100); //speed ADC value is mapped from 1 to 100
 
  fncUdpSend(speed_adc, direction_adc); //call send function
  delay(50);// wait 50 ms and send the next packet
}
