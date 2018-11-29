/*
  DHCP-based IP printer

  This sketch uses the DHCP extensions to the Ethernet library
  to get an IP address via DHCP and print the address obtained.
  using an Arduino Wiznet Ethernet shield.

  Circuit:
   Ethernet shield attached to pins 10, 11, 12, 13

  created 12 April 2011
  modified 9 Apr 2012
  by Tom Igoe
  modified 02 Sept 2015
  by Arturo Guadalupi

*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xDE, 0xBB, 0xCC, 0xDE, 0x02
};
//const char* server = "192.168.30.200";


const int button1 = 2;     // the number of the pushbutton pin
const int button2 = 3;      
const int led1 = 7;
const int led2 = 6;
const int rly1 = 8;
const int rly2 = 9;

int buttonState1 = HIGH;
int buttonState2 = HIGH;
int reading1;
int reading2;
int previous1 = LOW;
int previous2 = LOW;
int mset1 = LOW;
int mset2 = LOW;

#define SWITCH1 "/azana/ruang_kaca/switch1/set"
#define SWITCH2 "/azana/ruang_kaca/switch2/set"

unsigned long time = 0;
unsigned long debounce = 200UL;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
EthernetClient client;
PubSubClient mqttClient(client);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (strcmp(topic, SWITCH1)==0)
    {//do stuff here because it was a match
      if (strncmp(payload,"ON",length)==0){
        Serial.print("IT's 1 ON");
        mset1 = HIGH;
        } else if (strncmp(payload, "OFF",length)==0) {
          Serial.print("IT's 1 OFF");  
          mset1 = LOW;
        }
    }   

  if (strcmp(topic,SWITCH2)==0)
    {//do stuff here because it was a match
      if (strncmp(payload,"ON",length)==0){
        Serial.print("IT's 2 ON");
        mset2 = HIGH;
        } else if (strncmp(payload, "OFF",length)==0) {
          Serial.print("IT's 2 OFF");
          mset2 = LOW;
        }
    }   
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("outTopic","hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
      mqttClient.subscribe("/azana/ruang_kaca/switch1/set");
      mqttClient.subscribe("/azana/ruang_kaca/switch2/set");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
 

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // this check is only needed on the Leonardo:
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(rly1, OUTPUT);
  pinMode(rly2, OUTPUT);
  
  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  }
  // print your local IP address:
  printIPAddress();
  mqttClient.setServer("192.168.30.200", 1883);
  
//  if (mqttClient.connect("ruang_kaca")) {
//    // connection succeeded
//    Serial.print("MQTT Connected");
//  } else {
//    // connection failed
//    // mqttClient.state() will provide more information
//    // on why it failed.
//    Serial.print("MQTT Failed");
//  }

  mqttClient.setCallback(callback);

}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  
  mqttClient.loop();
  
  reading1 = digitalRead(button1);
  reading2 = digitalRead(button2);

  if (reading1 == HIGH && previous1 == LOW && millis() - time > debounce)
  {
    if (buttonState1 == HIGH) {
      buttonState1 = LOW;
      mset1 = LOW;
      digitalWrite(led1, LOW);
      digitalWrite(rly1, LOW);
    }     
    else { 
      buttonState1 = HIGH;
      mset1 = HIGH;
      digitalWrite(led1, HIGH);
      digitalWrite(rly1, HIGH);
    }  
     boolean rc = mqttClient.publish("/azana/ruang_kaca/switch1", mset1==HIGH?"ON":"OFF");
     time = millis(); 
  }

  if (reading2 == HIGH && previous2 == LOW && millis() - time > debounce)
  {
    if (buttonState2 == HIGH) {
      buttonState2 = LOW;
      mset2 = LOW;
      digitalWrite(led2, LOW);
      digitalWrite(rly2, LOW);
    }     
    else
    { 
      buttonState2 = HIGH;
      mset2 = HIGH;
      digitalWrite(led2, HIGH);
      digitalWrite(rly2, HIGH);
      }  
      boolean rc = mqttClient.publish("/azana/ruang_kaca/switch2", mset2==HIGH?"ON":"OFF");
      time = millis(); 
  }

  if (mset1 != buttonState1){
     buttonState1 = mset1;
     digitalWrite(led1, buttonState1);
     digitalWrite(rly1, buttonState1);
     boolean rc = mqttClient.publish("/azana/ruang_kaca/switch1", mset1==HIGH?"ON":"OFF");
  }

  if (mset2 != buttonState2){
     buttonState2 = mset2;
     digitalWrite(led2, buttonState2);
     digitalWrite(rly2, buttonState2);
     boolean rc = mqttClient.publish("/azana/ruang_kaca/switch2", mset2==HIGH?"ON":"OFF");
  }
  
  previous1 = reading1;
  previous2 = reading2;

}



void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}
