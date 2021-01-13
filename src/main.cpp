#include <Arduino.h>
#include<SoftwareSerial.h>
#include<EEPROM.h>


#define Hall_Effect_Sensor_pin 2
// #define Reed_Switch_pin 2


// this pin will be attached to a wire of which one end will be high
// this is used for detecting if the sensor was cut
// when the sensor wire is cut, this wire will also be cut
// thus generating a signal
#define wire_cut_detect_pin 3

// this pin will be used for hardware reset
#define reset_pin 1

String Gateway_Number = "01313791040";

SoftwareSerial SIM800L(8, 9); // new (Rx, Tx) of pro mini

// For sending message
void SendMessage(String message, String number)
{
  SIM800L.println("AT+CMGF=1");    
  delay(1000);
  SIM800L.println("AT+CMGS=\"+" + number + "\"\r"); 
  delay(1000);
  SIM800L.println(message);
  delay(100);
  SIM800L.println((char)26);
  delay(1000);
}

// For receiving message
String readMessage()
{
//   SIM800L.println("AT+CNMI=1,1,0,0,0"); 
//   delay(1000);

  SIM800L.println("AT+CMGL=\"REC UNREAD\"\r"); // To get the unread message

  //SIM800L.write("AT+CMGL=\"ALL\"\r"); // For All stored messages
  //Delete("8");
  //SIM800L.write("AT+CMGR=1\r");
  String data;
  if(SIM800L.available()>0)
  {
    data = SIM800L.readString();
    Serial.println(data);
  }

  return data;
}

//Breakdown the messsage into:
//


String Message_Sender_Number = "";
String Message_Time_Stamp = "";
String Message = "";

void Break_Message(String msg)
{
    int sender_number_index = msg.indexOf("+880");
    int time_stamp_index = sender_number_index + 22; //rough estimate
    int message_index = time_stamp_index + 15; // rough estimate

    Message_Sender_Number = msg.substring(sender_number_index, 14);
    Message_Time_Stamp = msg.substring(time_stamp_index, 14);
    Message = msg.substring(message_index);
}

void Delete(String location)
{
  SIM800L.println("AT+CMGD="+ location + "\r");
}

// this variable will count the meter pulses
// when a certain amount of water passes, the meter will send a pulse through reed switch
unsigned long int counter = 0;

// // this variable will save the total water flow
// unsigned long int total_water_flow = 0;


//ms before interrupt is detected again
// used to debounce
#define bounce_time 200
void pulse_counter()
{
    static unsigned long last_interrupt_time = 0;
    unsigned long interrupt_time = millis();

    if(interrupt_time - last_interrupt_time > bounce_time)
    {
        counter++;
    }

    last_interrupt_time = interrupt_time;
  
}


void reset()
{
    // counter and total water will be reseted
}


void setup() {
  Serial.begin(9600);
  SIM800L.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  
  SIM800L.println("AT+CNMI=1,1,0,0,0"); 
  delay(1000);
  pinMode(Hall_Effect_Sensor_pin, INPUT_PULLUP);
    // pinMode(Reed_Switch_pin, INPUT_PULLUP);
  attachInterrupt(Hall_Effect_Sensor_pin, pulse_counter, FALLING);
  
}

void loop() {

}

