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

void switchLed(bool on)
{
  if(on)
  {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
  if(!mqttClient.publish("capteur/LedPower", String(on).c_str()) || !mqttClient.publish("capteur/BuzzerPower", String(on).c_str()))
  {
    Serial.println("Unable to publish led state value..");
  }
}

void actionCallback(char * topicChar, byte* payloadByte, unsigned int length)
{
  Serial.println("New message received");
  String topic = String(topicChar);
  String payload = payloadToString(payloadByte, length);

  Serial.print("Topic: ");
  Serial.println(topic);

  Serial.print("Payload: ");
  Serial.println(payload);

  if(!topic.equals("capteur"))
  {
    return;
  }

  if(payload.equalsIgnoreCase("on"))
  {
    switchLed(true);
  }
  else if(payload.equalsIgnoreCase("off"))
  {
    switchLed(false);
  }
}

void setupMqttClient() {
  mqttClient.setClient(ethClient);
  mqttClient.setServer(clientServer,clientPort);
  mqttClient.setCallback(actionCallback);
}


void subscribActionTopic(){
  if(!mqttClient.subscribe("capteur/LedPower") || !mqttClient.subscribe("capteur/BuzzerPower") )
    {
      Serial.println("Topic action subscribe error");
      return;
    }
  Serial.println("Topic led action subscribe success");
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

    if (mqttClient.connect("capteurMqtt", clientUser, clientPassword))
    {

      Serial.println("connected success");
      subscribActionTopic();
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
}


void loop()
{
  publishDistance();  
  delay(5000);
  mqttClient.loop();
}
