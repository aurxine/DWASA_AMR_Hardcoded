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



unsigned long int current_water_meter_reading = 0; // reading on the meter
int meter_reading_save_address = 0; // starting address of EEPROM where meter reading will be saved
int flow_per_pulse_save_address = sizeof(unsigned long int); // starting address of EEPROM where flow per pulse will be saved
int flow_per_pulse; // water flow in liters per pulse

String Gateway_Number = "01313791040"; // Number to which all communication will occur

SoftwareSerial SIM800L(9, 8); // new (Rx, Tx) of pro mini

void Delete_Message(String location)
{
  SIM800L.println("AT+CMGD="+ location + "\r");
}

// For sending message
void Send_Message(String message, String number)
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
String Read_Message()
{
  // SIM800L.println("AT+CNMI=1,1,0,0,0"); 
  // delay(1000);

  // SIM800L.println("AT+CMGL=\"REC UNREAD\"\r"); // To get the unread message

  // SIM800L.write("AT+CMGL=\"ALL\"\r"); // For All stored messages
  SIM800L.write("AT+CMGR=1\r");
  String data ;//= "";
  if(SIM800L.available()>0)
  {
    data = SIM800L.readString();
    // Serial.println(data);
  }
  Delete_Message("1");

  return data;
}

//Breakdown the messsage into:
//


String Message_Sender_Number = "";
String Message_Time_Stamp = "";
String Message = "";

void Break_Message(String msg)
{
    int sender_number_index = 58;//msg.indexOf("+880");
    int time_stamp_index = sender_number_index + 20; //rough estimate
    int message_index = time_stamp_index + 20; // rough estimate

    Message_Sender_Number = msg.substring(67, 80);
    Message_Time_Stamp = msg.substring(87, 106);
    Message = msg.substring(110);
}

void reset()
{
  Serial.println("Reseting");
  current_water_meter_reading = 0;
  counter = 0;
  EEPROM.put(meter_reading_save_address, current_water_meter_reading);
    // counter and total water will be reseted
}

bool Execute_Command(String Command)
{
  if(Command.indexOf("meter") > -1)//Command looks like this meter:xxxxxx
  {
    String initial_water_meter_reading_str = Command.substring(6);
    current_water_meter_reading = initial_water_meter_reading_str.toInt();
    EEPROM.put(meter_reading_save_address, current_water_meter_reading);
    Serial.println(current_water_meter_reading);
    return true;
  }
  else if(Command.indexOf("flow") > -1)//Command looks like this flow:xxxxxx
  {
    String flow_per_pulse_str = Command.substring(5);
    flow_per_pulse = flow_per_pulse_str.toInt();
    EEPROM.put(flow_per_pulse_save_address, flow_per_pulse);
    Serial.println(flow_per_pulse);
    return true;
  }
  else if(Command.indexOf("getwater") > -1) // Command looks like ***getwater***
  {
    // send water meter reading
    Send_Message(String(current_water_meter_reading), Gateway_Number);
    return true;
  }
  else if(Command.indexOf("reset") > -1) // Command looks like reset
  {
    reset();
    return true;
  }
  else
  {
    return false;
  }
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
        current_water_meter_reading += flow_per_pulse;
        EEPROM.put(meter_reading_save_address, current_water_meter_reading); 
    }

    last_interrupt_time = interrupt_time;
  
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
  // bool check = Execute_Command("meter:1233141");
  // Serial.println(check);
  // check = Execute_Command("flow:100");
  // Serial.println(check);
  EEPROM.get(meter_reading_save_address, current_water_meter_reading);
  EEPROM.get(flow_per_pulse_save_address, flow_per_pulse);

  Serial.println(current_water_meter_reading);
  Serial.println(flow_per_pulse);

  //Send_Message("Hello", "01624593436");
  // String msg = "";
  // msg = Read_Message();
  // Serial.println(msg);
  // delay(3000);
  // msg = Read_Message();
  // Serial.println(msg);
  // msg = Read_Message();
  // Serial.println(msg);

  
}

void loop() {
    // String msg = "";
    // msg = Read_Message();
    // for(int i = 0; i< 10; i++)
    // {
    //   Serial.print("#");
    // }
    // // Serial.println();
    // // Serial.println(msg);
    // // msg.replace("\n", "*");
    // for(int i = 0; i < msg.length(); i++)
    // {
    //   Serial.print(i);
    //   Serial.print(" :");
    //   Serial.println(msg[i]);
    // }
    // //Serial.println(msg);

    // // Break_Message(msg);
    // // Serial.print("Number: ");
    // // Serial.print(Message_Sender_Number);
    // // Serial.print(" Time Stamp: ");
    // // Serial.print(Message_Time_Stamp);
    // // Serial.print(" Message: ");
    // // Serial.println(Message);
    // for(int i = 0; i< 10; i++)
    // {
    //   Serial.print("#");
    // }
    // Serial.println();
    
    // delay(3000);

}

