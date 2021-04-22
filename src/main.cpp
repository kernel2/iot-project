#include <Arduino.h>
#include <stdlib.h>
#include <PubSubClient.h>
#include <EthernetClient.h>

#include <Ethernet.h>

#define ECHO_PIN 11
#define TRIG_PIN 12

const int LED_PIN = 7;
const int BUZZER_PIN = 3;

float time_echo = 0;
float distance = 0;

const char *clientServer = "10.0.2.2";
const int clientPort = 1883;
const char *clientUser = "admin";
const char *clientPassword = "admin";

Ethernet WSA;
EthernetClient ethClient;
PubSubClient mqttClient;


String payloadToString(byte *payload, unsigned int length)
{
  char buffer[length];
  sprintf(buffer, "%.*s", length, payload);
  return String(buffer);
}


void callback(char *topicChar, byte *payload, unsigned int length)
{
  String message = payloadToString(payload, length);
  String topic = String(topicChar);
  if(!topic.equals("captureLED")){
    return;
  } else if(topic.equals("captureLED")){
        if (message.equals("on"))
        {
          digitalWrite(LED_PIN, HIGH);
          digitalWrite(BUZZER_PIN, HIGH);
        } else if (message.equals("off")) {
          digitalWrite(LED_PIN, LOW);
          digitalWrite(BUZZER_PIN, LOW);
        } 
    }
}

void setupMqttClient() {
  mqttClient.setClient(ethClient);
  mqttClient.setServer(clientServer,clientPort);
  mqttClient.setCallback(callback);
}


void setup()
{
  Serial.begin(9600);

  //capteur de distance
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  setupMqttClient();

  while (!mqttClient.connected())
  {
    Serial.println("Connecting to MQTT client...");

    if (mqttClient.connect("captureMqtt", clientUser, clientPassword))
    {

      Serial.println("connected");
      if (mqttClient.subscribe("capteur/distance"))
      {
        Serial.println("subscribed to the capteur/distance");
      }
    }
    else
    {

      Serial.print("failed with state ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}


void publishDistance(){
  //time_echo = pulseIn(ECHO_PIN, HIGH, 1000);
  // distance en random
  int rand_distance = random(0, 100);
  distance = rand_distance;
  Serial.print("time_echo");

  mqttClient.publish("capteur/distance", String(distance).c_str());
  Serial.println("pushed on the client with success");
  if(distance <= 50){
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    mqttClient.publish("captureLED/ledStateTopic", String("on").c_str());
    mqttClient.publish("captureLED/buzzerStateTopic", String("on").c_str());
  }else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    mqttClient.publish("captureLED/ledStateTopic", String("off").c_str());
    mqttClient.publish("captureLED/buzzerStateTopic", String("off").c_str());
  }
  if(distance > 50 || distance <= 0){
    mqttClient.publish("captureOutRange/StateTopic", String("OutOfRangeDistance").c_str());
    Serial.println("Out of distance");
    Serial.print(distance);
    Serial.println(" cm");
  }else {
    Serial.print(distance);
    Serial.println(" cm");
  }  
}

void subscribeDistance(){
  mqttClient.subscribe("capteur/distance");
  Serial.println("subscribe Distance in Capture topic");
}

void loop()
{
  digitalWrite(TRIG_PIN, HIGH);
  delay(10);
  digitalWrite(TRIG_PIN, LOW);
  
  publishDistance();  
  delay(5000);
  subscribeDistance();
  mqttClient.loop();
}
