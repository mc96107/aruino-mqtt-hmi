/*
 Basic MQTT example

 This sketch demonstrates the basic capabilities of the library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic"
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include "DHT.h"
#define DHTPIN 2 
#define DHTTYPE DHT22 
unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
unsigned long lastDataTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 30*1000;  // delay between updates, in milliseconds
DHT dht(DHTPIN, DHTTYPE);
float tempValue = 0;
float h = 0;
int flag =1;
const int ledPin =  6; 
float ledState = 0;
// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 3);
IPAddress subnet(255, 255, 255, 0);
IPAddress DNS(192, 168, 1, 254);
IPAddress gw(192, 168, 1, 254);
IPAddress server(192, 168, 1, 106);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  payload[length] = '\0';
  String s = String((char*)payload);
  float f = s.toFloat();
  Serial.print(f);
  ledState=f;
  chledstate();
}

EthernetClient ethClient;
char servername[]="192.168.1.106";
PubSubClient client(servername, 1883, callback, ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  
  Serial.begin(57600);
  pinMode(ledPin, OUTPUT);
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip, dns, gw, subnet);
  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if((millis() - lastDataTime > postingInterval*6)) {
    readdht();
  }

  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if(flag) {flag=0;readdht(); }  
}

void mqtt(){
  String clientName = String("arduinoClient");
  String topicName = String("iot-2/evt/status/fmt/json");
  int str_len = clientName.length() + 1; 
  char clientStr[str_len];
  clientName.toCharArray(clientStr,str_len);
  str_len = topicName.length() + 1;
  char topicStr[str_len];
  topicName.toCharArray(topicStr,str_len);
  char token[] = "s?)5I_hclg&WcLf2yH";
  Serial.println(clientStr);
  if (!client.connected()) {
    Serial.print("Trying to connect to: ");
    Serial.println(clientStr);
    client.connect(clientStr, "use-token-auth", token);
  }
 if (client.connected() ) {
    String json = buildJson();
    str_len = json.length() + 1; 
    char jsonStr[str_len];
    json.toCharArray(jsonStr,200);
    boolean pubresult = client.publish(topicStr,jsonStr);
    Serial.print("attempt to send ");
    Serial.println(jsonStr);
    Serial.print("to ");
    Serial.println(topicStr);
    if (pubresult)
      Serial.println("successfully sent");
    else
      Serial.println("unsuccessfully sent");
    //client.subscribe("#");  
    client.loop();
  }
}

String buildJson() {
  String data = "{";
  data+="\n";
  data+= "\"d\": {";
  data+="\n";
  data+="\"t\": ";
  data+=(float)tempValue;
  data+= ",";
  data+="\n";
  data+="\"h\": ";
  data+=(int)h;
  data+= ",";
  data+="\n";
  data+="\"led\": ";
  data+=(float)ledState;
  data+="\n";
  data+="}";
  data+="\n";
  data+="}";
  return data;
}

void readdht(){
    tempValue = dht.readTemperature();
    h = dht.readHumidity();
    Serial.println();
    Serial.print("Temp : ");
    Serial.print(tempValue);
    Serial.print(" ; RH : ");
    Serial.print(h);
    lastDataTime= millis();
    mqtt();
}

void chledstate(){
    if(ledState==1.00) digitalWrite(ledPin, HIGH); 
  else if (ledState==0.00) digitalWrite(ledPin, LOW);
}

