/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// Update these with values suitable for your network.

const char* ssid = "KOO";
const char* password = "11111111";
const char* mqtt_server = "121.137.153.145";
const char* device_name = "led";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE   (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int hw_active = 0;  //When message arrived open(hw_active=0 -> 1) / close(hw_active=1 ->0)

static const int servoPin = 2;
Servo servo1;


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(33, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  char* c_payload = (char*)payload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  // Active(payload[0]-48);
  Serial.println();
  if (!strcmp(topic, "control_name")) {
    if (!strncmp(c_payload, device_name, strlen(device_name))) {
      Servo();
    }
  }


  // Serial.print("Active: ");
  // Serial.println(payload[0]);

}

// void Active(int device){

//   Serial.print("device: ");
//   Serial.println(device);
//   switch(device){
//     case 0: break;           //Window
//     case 1: break;           //Door
//     case 2: Servo(); break;  //LED
//   }

// }

void Servo(){
  Serial.println("Servo activate");
  digitalWrite(33, LOW);
  if(hw_active==0){
    Serial.println("ON");
    for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees); 
        Serial.println(posDegrees);
        delay(20);
    }
    hw_active = 1;
  }
  else{
    Serial.println("OFF");
    for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees); 
        Serial.println(posDegrees);
        delay(20);
    }
    hw_active = 0; 
  }
  digitalWrite(33, HIGH);
  return; 
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "LED_device";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("control_name");    //Todo subscribe control_name 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  digitalWrite(33, LOW);
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(33, OUTPUT);
  servo1.attach(servoPin);
  digitalWrite(33, HIGH);
  servo1.write(0); 
}

void loop() {
  
  char receivedData;
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  // if (now - lastMsg > 2000) {
  //   lastMsg = now;
  //   ++value;
  //   snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
  //   Serial.print("Publish message: ");
  //   Serial.println(msg);
  //   client.publish("outTopic", msg);
  // }
}