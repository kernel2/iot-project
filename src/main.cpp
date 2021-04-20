#include <PubSubClient.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
IPAddress server(172, 17, 0, 4);

#define LED_PIN D4

int trig = 12;
int echo = 13;
long lecture_echo;
long cm;
const char *ssid = "HUAWEI P20 lite"; //Your WiFI ssid
const char *password = "a1234567";    //Your WiFi password

const char *mqttServer = "172.17.0.4";
const int mqttPort = 1883;
const char *mqttUser = "admin";
const char *mqttPassword = "admin";

WiFiClient espClient;
PubSubClient client(espClient);

String payloadToString(byte *payload, unsigned int length)
{
  char buffer[length];
  sprintf(buffer, "%.*s", length, payload);
  return String(buffer);
}

void callback(char *topic, byte *payload, unsigned int length)
{
  String message = payloadToString(payload, length);
  if (message.equals("on"))
  {
    digitalWrite(LED_PIN, LOW);
  }
  else if (message.equals("off"))
  {
    digitalWrite(LED_PIN, HIGH);
  }
}

PubSubClient mqtt(server, 1883, callback, espClient);

void setup()
{
  Serial.begin(9600);

  //capteur de distance
  pinMode(trig, OUTPUT);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT);

  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected())
  {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client", mqttUser, mqttPassword))
    {

      Serial.println("connected");
      if (client.subscribe("capteur/led"))
      {
        Serial.println("subscribed to the capteur/led");
      }
    }
    else
    {

      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void reconnect()
{
  while (!mqtt.connected())
  {
    Serial.println("[MQTT] Connecting to MQTT...");
    if (mqtt.connect("MonObjectConnecte")) // Or mqtt.connect("MonObjectConnecte", "user", "pass") if you have protect the mqtt broker by credentials
    {
      Serial.println("[MQTT] Connected");
      mqtt.publish("EssaieIOT", "Hello world from WizIO");
      mqtt.subscribe("EssaieIOT");
    }
    else
    {
      Serial.print("[ERROR] MQTT Connect: ");
      Serial.println(mqtt.state());
      delay(60 * 1000);
    }
  }
}

void loop()
{
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  lecture_echo = pulseIn(echo, HIGH);
  cm = lecture_echo / 58;
  Serial.print("Distance en cm :");
  Serial.println(cm);
  client.publish("capteur/distance", String(cm).c_str(), true); //Publie la température sur le topic temperature_topic
  Serial.println("pushed on the client with success");
  delay(1000);
  client.loop();
}
