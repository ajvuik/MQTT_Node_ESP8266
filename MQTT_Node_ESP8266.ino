/***************************************************************************************
* Name        : MQTT_Node_ESP8266
* Author      : Arjan Vuik 
* Description : With this an ESP8266 logs in to an MQTT broker and publishes
*               a DS18b20 temperature on the defined channel. the ESP wil subscribe
*               to the difined channels and relay the messages from those channels to 
*               the serial port. 
****************************************************************************************/

#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <stdlib_noniso.h>

//Define the pin number on which the onewire bus is connected 
#define ONE_WIRE_BUS 2

//Network config for the ESP
const char* ssid = "YOURSSIDHERE";
const char* password = "YOURSUPERPASSWORDHERE";

//Difine the MQTT broker adress
byte server[] = {192,168,2,2};

//Add your topics to publish to here
char topic1111[] = "/1111";

//Add your topics to subscribe here
char topic1121[] = "/1121";
char topic1122[] = "/1122";

//Topic handler
char topicRcv[] = "/0000";

//Payload handler
char payloadRcv[5];

//Change the clientname to your liking
String clientName = "Node1";
unsigned long PreviousMillis = 0;

//Setup the OneWireBus
OneWire oneWire(ONE_WIRE_BUS);
//Setup the DS18b20
DallasTemperature sensors(&oneWire);
//Setup the MQTT client
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient); 

//Incomming messages from the MQTT broker are handled here
void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  memcpy(&topicRcv, topic, sizeof(topicRcv)-1);
  memcpy(&payloadRcv,payload,length);
}


void setup() {
  //Setup serial port
  Serial.begin(115200);
  //Login to your wifi AP
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  //Enable the sensor(s)
  sensors.begin();
  //Connect to the broker
  client.connect((char*) clientName.c_str());
  //Subscribe to your topics. Add a subscribe for every topic you want.
  client.subscribe(topic1121);
  client.subscribe(topic1122);
  Serial.println("Node OK");
} 
 
void loop() {
  delay(50);//This is here to give the ESP time to handle WIFI tasks
  if(WiFi.status() == WL_CONNECTED){
    unsigned long CurrentMillis=millis();
    client.loop();//This keeps the connection to the broker alive
    //Get and publish the DS18b20 temp every 10 seconds
    if(CurrentMillis-PreviousMillis>10000){
      PreviousMillis=CurrentMillis;
      sensors.requestTemperatures();
      char charVal[10];
      String payload = "";
      float Temp = sensors.getTempCByIndex(0);
      dtostrf(Temp, 4, 1, charVal);
    
      for(int i=0;i<5;i++)
      {
        payload+=charVal[i];
      }
      charVal[5]='\0';
      client.publish(topic1111, (char*) payload.c_str());
    }
    //If something was published on one of our subscribed channels
    //Handle is here.
    if (!strstr(topicRcv,"/0000")){
      String Payload = topicRcv;
      Payload += ":"; 
      Payload += payloadRcv;
      Serial.println(Payload);
      topicRcv[0]='/';
      topicRcv[1]='0';
      topicRcv[2]='0';
      topicRcv[3]='0';
      topicRcv[4]='0';
    }
  }
}
 
  
