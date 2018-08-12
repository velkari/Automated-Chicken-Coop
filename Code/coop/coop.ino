

#define feed_sensor A0
#define tank_sensor A1
#define egg_sensor A2

#include <Servo.h>
Servo myservo;
int pos = 0; 
char remoteNum[20]="0789791393";
// Include the GSM library
#include <GSM.h>
char senderNumber[20];
#define PINNUMBER ""

// initialize the library instance
GSM gsmAccess;
GSM_SMS sms;
char c;
#include <DS3231.h>
int ledpin=13;
// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);
Time t;
int f;

int feedmotorpin1=20;
int feedmotorpin2=21;
int feedenablePin1=22;
int wastemotorpin1=15;
int wastemotorpin2=16;
int wasteenablePin1=17;

int EggCount=0;

void setup()
{
  myservo.attach(9);
  Serial.begin(115200);
  rtc.begin();
  Serial.println("Automated chicken coop: FYP-14-16-");

  // connection state
  boolean notConnected = true;

  // Start GSM shield
  // If your SIM has PIN, pass it as a parameter of begin() in quotes
  while (notConnected)
  {
    if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
        notConnected = false;
    } 
    else 
    {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("GSM initialized");
}



void loop()
{
  
receiveMessage();
FeedLevel();
TankLevel();
rtc.getTimeStr();
t = rtc.getTime();
int Hor= t.hour;
int Min= t.min;
int Sec= t.sec;
while (Hor == 07||13 && Min==00 && Sec ==30)
  {
  FeedTime();
  break;
  }
    while (Hor == 18 && Min==00 && Sec ==30)//waste is  cleaned up at 6pm. 
      {
       wasteremoval();  
       break;
      }
         while (Hor == 20 && Min==00 && Sec ==30)//final eggcount is sent at 8pm. 
         {
          sendMessage();  
          break;
         }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FeedLevel()
{
  float volts = analogRead(feed_sensor)*0.0048828125;  // value from sensor * (5/1024)                                                                                                     
  int distance_feed = 2*13*pow(volts, -1); // worked out from datasheet graph
  delay(50); // slow down serial port 
  
  while (distance_feed <= 50)
   {
    //send msg to refill silo
    char feedMsg[200]="Feed level is low. You need to refill it!";
    // send the message
  sms.beginSMS(remoteNum);
  sms.print(feedMsg);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n"); 
  break;
    }
}
  ///////////////////////////////////////////////////////////////////////////////////////////////
  void TankLevel()
{
  float volts1 = analogRead(tank_sensor)*0.0048828125;  // value from sensor * (5/1024)                                                                                                     
  int distance_tank = 2*13*pow(volts1, -1); // worked out from datasheet graph
  delay(50); // slow down serial port 
  
  while (distance_tank <= 50)
  {
    //send msg to refill tank
    char waterMsg[200]="Water level is low. You need to refill it!";
    // send the message
  sms.beginSMS(remoteNum);
  sms.print(waterMsg);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n"); 
  break;
  } 
}
 ///////////////////////////////////////////////////////////////////////////////////////////////////////// 

  void Eggcounter()
{
  float volts2 = analogRead(tank_sensor)*0.0048828125;  // value from sensor * (5/1024)                                                                                                     
  int distance_tank = 2*13*pow(volts2, -1);             // worked out from datasheet graph
  delay(50);                                            // slow down serial port 
  
  while (distance_tank <= 50)
  {
    EggCount+=1;
    delay(200);
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  void FeedTime()
 {
    for (pos = 0; pos <= 90; pos += 1) 
    {                                    // goes from 0 degrees to 180 degrees in steps of 1 degree
    myservo.write(pos);                  // tell servo to go to position in variable 'pos'
    delay(5000);                         // waits 5000s for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1)
    {                                   // goes from 180 degrees to 0 degrees
    myservo.write(pos);                 // tell servo to go to position in variable 'pos'
    delay(500);                         // waits 500ms for the servo to reach the position
    }
  digitalWrite(feedenablePin1,HIGH);
  digitalWrite( feedmotorpin1,HIGH); // start motor to move cabin forwards
  digitalWrite( feedmotorpin2,LOW);
  delay(10000);
  digitalWrite(feedenablePin1,HIGH);
  digitalWrite( feedmotorpin1,LOW); //change direction of motor to move the cabin ot original position
  digitalWrite( feedmotorpin2,HIGH);
  delay(10000);
  digitalWrite(feedenablePin1,LOW);
  digitalWrite( feedmotorpin1,LOW); //turn off motor
  digitalWrite( feedmotorpin2,LOW);
   
 }

 //////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void wasteremoval()
 {
      digitalWrite(wasteenablePin1,HIGH);
      digitalWrite( wastemotorpin1,HIGH); // start motor to move the conveyor
      digitalWrite( wastemotorpin2,LOW);
      delay(10000);
      digitalWrite(wasteenablePin1,LOW);
      digitalWrite( wastemotorpin1,LOW); // turn off motor
      digitalWrite( wastemotorpin2,LOW);
 }
    

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
 void receiveMessage()
 {
  // If there are any SMSs available()
  if (sms.available()) 
  {
    Serial.println("Message received from:");
    // Get remote number
    sms.remoteNumber(senderNumber, 20);
    Serial.println(senderNumber);

   while (sms.peek() == 'Q') 
    {
      while(c = sms.read())
      {
       Serial.println(c); 
       Serial.println("SENDING TO:");
       Serial.println(senderNumber);
       Serial.println();
       Serial.println("Message:");
       Serial.println(rtc.getTimeStr());
       delay(1000);
         sms.beginSMS(senderNumber);
         sms.print("Number of eggs collected is ");
         sms.print(EggCount);
         sms.endSMS();
         Serial.println("\nSMS SENT. SYSTEM QUERRIED!\n"); 
         Serial.println("\nEND OF MESSAGE");
         break;
       }
    // Delete message from modem memory
    sms.flush();
    Serial.println("MESSAGE DELETED");
    delay(1000);
    break;
     }
 
    Serial.println(c); 
    sms.flush();
    Serial.println("SEND 'Q' TO QUERRY SYSTEM");
    delay(1000);
    break;
    }
  }

  void sendMessage()
{
  char remoteNum[20]="0789791393";  // telephone number to send sms
//  readSerial(remoteNum);
  Serial.println(remoteNum);
  // sms text
  char txtMsg[200]="Total number of eggs collected today are: ";
//  readSerial(txtMsg);
  Serial.println("SENDING");
  Serial.println();
  Serial.println("Message:");
  Serial.println(txtMsg);

  // send the message
  sms.beginSMS(remoteNum);
  sms.print(txtMsg);
  sms.print(EggCount);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n"); 
 }

  
  
