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
// this variable will count the meter pulses
// when a certain amount of water passes, the meter will send a pulse through reed switch
unsigned long int counter = 0;

// structur for incoming message and sender's number
typedef struct SString{
  String number;
  String text;
};

String Gateway_Number = "01313791040"; // Number to which all communication will occur
SString Message; // this stores the sender's number and the text
bool new_message = false; //checks if there is a new message

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
SString Receive_Message()
{
    String textMessage = "";

    //SIM800L.print("AT+CMGF=1\r");
    //delay(1000);
    SIM800L.println("AT+CNMI=1,2,0,0,0\r");  
    delay(2000);

    // SIM800L.println("AT+CMGL=\"REC UNREAD\"\r"); // to get the unread message
    SIM800L.println("AT+CMGF=1\r"); // to get the message stored at location 1
    if(SIM800L.available() > 0)
    {
        textMessage = SIM800L.readString();
        delay(500);
        Serial.println(textMessage);
        // Delete_Message("1");
    }
    // Serial.println(textMessage);

    SString msg;
    String temp;
    String data = textMessage;
    int len = data.length(), cnt = 0, cnt1 = 0, cnt2 = 0;

    bool next_rn = false;

    for(int i = 0; i < len;i++)
    {
      // Serial.println(data.substring(i,i+1));
      temp = data.substring(i,i+1);

      if(temp == "+" && i < len - 1)
      {
        // Serial.println(data.substring(i+1,i+2));
        if(data.substring(i+1,i+2) == "8")
        {
          msg.number = data.substring(i,i+14);
        }
            
      }

      else if(temp == "\"" && i < len -1)
      {
        if(data.substring(i + 1, i + 2) == "\r" && data.substring(i + 2, i + 3) == "\n")
        {
          cnt1 = i + 3;
          next_rn = true;
          i += 3;
        }
        
      }

      else if(temp == "\r" && i < len -1)
      {
        if(data.substring(i + 1, i + 2) == "\n" && next_rn)
        {
          cnt2 = i;
          // Serial.print("Extracted text: ");
          // Serial.println(data.substring(cnt1, cnt2));
          next_rn = false;
        }
      }


        // if(temp == "\n")
        // {
        //   Serial.print(i);
        //   Serial.println(" : *n*");
        // }
        // else if(temp == "\r")
        // {
        //   Serial.print(i);
        //   Serial.println(" : *r*");
        // }
        // else if(temp == "\"")
        // {
        //   Serial.print(i);
        //   Serial.println(" : *q*");
        // }

        // else
        // {
        //   Serial.print(i);
        //   Serial.print(" : ");
        //   Serial.println(temp);
        // }
      
            
    }
    msg.text = data.substring(cnt1, cnt2);
    // Serial.println(msg.number);
    // Serial.println(msg.text);

    return msg;
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
    Send_Message("getwater:" + String(current_water_meter_reading), Gateway_Number);
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

  
  SIM800L.println("AT+CNMI=1,2,0,0,0"); 
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
  
}

void loop() 
{
  Message = Receive_Message();

  Serial.print("Number: ");
  Serial.print(Message.number);
  Serial.print(" Text: ");
  Serial.println(Message.text);

  // if(new_message)
  // {
  //   Serial.print("Received new message!\n");
  //   Serial.print(Message.number);
  //   Serial.println(Message.text);
  // }
  // else
  // {
  //   Serial.println("No new message was received");
  // }
  delay(1000);
}

